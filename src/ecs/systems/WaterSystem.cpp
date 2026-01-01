#include "WaterSystem.h"
#include "../../algorithm/compute/noise/Gaussian.h"
#include "../../ui/Ui.h"
#include "../../renderer/base/Model.h"


namespace EnGl
{
	static void CascadeView(WaterSystem::Cascade& cascade, u32 i)
	{
		auto& data = cascade.m_SpectrumData;

		ImGui::InputFloat("Lambda", &data.Lambda);

		ImGui::InputFloat("TimeScale", &data.TimeScale);
		ImGui::InputFloat("Tiling", &data.Tiling);

		
		data.ParametersChanged |= ImGui::InputFloat("L", &data.L);
		data.ParametersChanged |= ImGui::InputFloat("LowCutoff", &data.LowCutoff);
		data.ParametersChanged |= ImGui::InputFloat("HighCutoff", &data.HighCutoff);
	}

	void WaterSystem::Editor(EcsImpl::EntityManager& manager, GameContext& context)
	{
		ImGui::Separator();

		ImGui::SliderFloat("FoamAdd", &m_Data.FoamAdd, 0.0f, 3.0f);
		ImGui::SliderFloat("FoamDecay", &m_Data.FoamDecay, 0.0f, 1.0f);

		bool spectrumChanged = false;
		spectrumChanged |= ImGui::InputFloat("Fetch", &m_Data.Fetch);
		spectrumChanged |= ImGui::SliderFloat("Swell", &m_Data.Swell, 0.0f, 1.0f);
		spectrumChanged |= ImGui::InputFloat("Depth", &m_Data.Depth);

		spectrumChanged |= ImGui::SliderFloat("WindAngleDegree", &m_Data.WindAngleDegree, 0.0f, 359.99f);
		spectrumChanged |= ImGui::InputFloat("WindSpeed", &m_Data.WindSpeed);

		int powof2 = glm::log2(static_cast<f32>(m_Data.N));
		if (ImGui::SliderInt("Power of 2 for N", &powof2, 0, 12))
		{
			m_Data.N = 1 << powof2;
			m_Data.DimensionChanged = true;
		}

		ImGui::Separator();
		for (size_t i = 0; i < WaterSystem::NCascades; i++)
		{
			ImGui::PushID(static_cast<int>(i));

			ImGui::Separator();
			ImGui::Text("Cascade %zu", i);
			ImGui::Spacing();

			CascadeView(m_Cascades[i], i);

			m_Cascades[i].m_SpectrumData.ParametersChanged |= spectrumChanged;

			ImGui::Spacing();
			ImGui::Separator();

			ImGui::PopID();
		}
	}

	static u32 GetGroupCount(u32 N)
	{
		return static_cast<u32>(glm::ceil(N / 16.0));
	}

	static u32 GetNStages(u32 N)
	{
		return static_cast<u32>(glm::ceil(glm::log2(static_cast<f32>(N))));
	}

	static AssetHandle<Texture2D> GenRG32FTexture(u32 N)
	{
		return AssetManager::Put<Texture2D>(
			N, N, Texture::CreationInfoFromData
			{
				.CpuFormat = GL_RG,
				.GpuFormat = GL_RG32F,
				.Common = 
					{
						.MinFilter = GL_NEAREST,
						.MagFilter = GL_NEAREST
					}
			}
		);
	}

	static AssetHandle<Texture2D> GenRGBA32FTexture(u32 N)
	{
		return AssetManager::Put<Texture2D>(
			N, N, Texture::CreationInfoFromData
			{
				.CpuFormat = GL_RGBA,
				.GpuFormat = GL_RGBA32F,
				.Common =
				{
					.MinFilter = GL_LINEAR_MIPMAP_LINEAR,
					.MagFilter = GL_LINEAR
				}
			}
		);
	}

	static AssetHandle<Texture2D> GenR32FTexture(u32 N)
	{
		return AssetManager::Put<Texture2D>(
			N, N, Texture::CreationInfoFromData
			{
				.CpuFormat = GL_RED,
				.GpuFormat = GL_R32F,
				.Common =
				{
					.MinFilter = GL_LINEAR_MIPMAP_LINEAR,
					.MagFilter = GL_LINEAR
				}
			}
		);
	}

	static FFT::IFFT2Dinfo GenInfo(u32 N)
	{
		return
		{
			.Ping = GenRG32FTexture(N),
			.Pong = GenRG32FTexture(N),
			.N = N,
			.Nstages = GetNStages(N),
			.GroupCount = GetGroupCount(N)
		};
	}

	std::vector<WaterSystem::Cascade> WaterSystem::GenCascades()
	{
		std::vector<Cascade> cascades;

		for (size_t i = 0; i < NCascades; i++)
			cascades.emplace_back(m_SpectrumData[i], m_Data, m_FFT);

		return cascades;
	}

	WaterSystem::WaterSystem(std::vector<SpectrumData> data) :
		m_SpectrumData(std::move(data)),
		m_Cascades(GenCascades())
	{
		assert(NCascades == m_Cascades.size());
	}

	struct WaterSurface : public Material::Base
	{
		WaterSystem& m_WaterSystem;

		AssetHandle<Texture2D> m_FoamTex = AssetManager::Load<Texture2D>(AssetManager::TEXTURE_DIR / "foam" / "foam.jpg");

		WaterSurface(WaterSystem& system) : m_WaterSystem(system), Base(AssetManager::GRAPHICS_SHADER_DIR / "WaterSurface") {}

		bool SetCommonUniforms(const GameContext& context) override
		{
			bool ok = Base::SetCommonUniforms(context);
			if (!ok)
			{
				return ok;
			}

			auto cam = context.Camera.Get();
			m_Shader->SetUniform("uViewProjection", cam.ViewProjection);
			m_Shader->SetUniform("uCameraPos", *cam.Position);
			m_Shader->SetUniform("uNear", cam.Near);
			m_Shader->SetUniform("uFar", cam.Far);
			m_Shader->SetUniform("uResolution", context.Framebuffer.Resolution);
			m_Shader->SetUniform("uTime", static_cast<f32>(context.Time));

			if (context.Cubemap)
			{
				m_Shader->SetUniform("uCubemap", *context.Cubemap, 0);
			}

			auto [foam, g] = AssetManager::GetAsset(m_FoamTex);
			auto [depth, g2] = AssetManager::GetAsset(context.Framebuffer.DepthTextureOpaque);
			if (foam && depth)
			{
				m_Shader->SetUniform("uFoamDetail", *foam, 1);
				m_Shader->SetUniform("uDepth", *depth, 2);
			}

			for (u32 i = 0; i < WaterSystem::NCascades; i++)
			{
				auto [dispTex, g0] = AssetManager::GetAsset(m_WaterSystem.m_Cascades[i].m_Displacement);
				auto [normalTex, g1] = AssetManager::GetAsset(m_WaterSystem.m_Cascades[i].m_Normal);

				if (!dispTex || !normalTex)
				{
					continue;
				}

				m_Shader->SetUniform("uCascades[" + std::to_string(i) + "].Displacement", *dispTex, (2 * i + 3));

				m_Shader->SetUniform("uCascades[" + std::to_string(i) + "].Normal", *normalTex, (2 * i + 4));


				m_Shader->SetUniform("uCascades[" + std::to_string(i) + "].N", m_WaterSystem.m_Cascades[i].m_CommonData.N);
				m_Shader->SetUniform("uCascades[" + std::to_string(i) + "].L", m_WaterSystem.m_Cascades[i].m_SpectrumData.L);
				m_Shader->SetUniform("uCascades[" + std::to_string(i) + "].Tiling", m_WaterSystem.m_Cascades[i].m_SpectrumData.Tiling);
				m_Shader->SetUniform("uCascades[" + std::to_string(i) + "].InvTiling", 1 / m_WaterSystem.m_Cascades[i].m_SpectrumData.Tiling);
				m_Shader->SetUniform("uCascades[" + std::to_string(i) + "].FoamScale", m_WaterSystem.m_Cascades[i].m_CommonData.FoamAdd);
				m_Shader->SetUniform("uCascades[" + std::to_string(i) + "].FoamFlatSubtract", m_WaterSystem.m_Cascades[i].m_CommonData.FoamDecay);
			}

			return ok;
		}
	};

	void WaterSystem::Init(EcsImpl::EntityManager& manager)
	{
		waterSurface = manager.Create<
			Component::Transform, Component::ModelMatrix, Component::RenderedModel
		>(
			[=](Component::Transform& transform, auto&, Component::RenderedModel& model) -> void
			{
				auto mat = AssetManager::Put<scope<Material::Base>>(make_scope<WaterSurface>(*this));
				model.Model = StaticModel::QuadTesselated(mat, 200, 200);
				model.Layer = Component::RenderLayer::TT;
				transform.Rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
				transform.Scale = glm::vec3{9000.0f};
			}
		);
	}

	void WaterSystem::Cascade::Init(u32 N)
	{
		m_IFFTDYCOMBINEDDXZ = GenInfo(N);
		m_IFFTDXCOMBINEDZ = GenInfo(N);
		m_IFFTDDXCOMBINEDDZ = GenInfo(N);
		m_IFFTDYDXCOMBINEDYDZ = GenInfo(N);
		m_Gaussian = Noise::Gaussian::Get(N);
		m_Spectrum = GenRG32FTexture(N);
		m_ConjSpectrum = GenRG32FTexture(N);
		m_GroupCount = GetGroupCount(N);
		m_NStages = GetNStages(N);
		m_dycombineddxz = m_IFFTDYCOMBINEDDXZ.Ping;
		m_dxcombinedz = m_IFFTDXCOMBINEDZ.Ping;
		m_ddxcombineddz = m_IFFTDDXCOMBINEDDZ.Ping;
		m_dydxcombinedydz = m_IFFTDYDXCOMBINEDYDZ.Ping;
		m_Normal = GenRGBA32FTexture(N);
		m_Displacement = GenRGBA32FTexture(N);
	}

	void WaterSystem::Cascade::Cleanup()
	{
		AssetManager::Remove(m_IFFTDYCOMBINEDDXZ.Ping);
		AssetManager::Remove(m_IFFTDYCOMBINEDDXZ.Pong);
		AssetManager::Remove(m_IFFTDXCOMBINEDZ.Ping);
		AssetManager::Remove(m_IFFTDXCOMBINEDZ.Pong);
		AssetManager::Remove(m_IFFTDDXCOMBINEDDZ.Ping);
		AssetManager::Remove(m_IFFTDDXCOMBINEDDZ.Pong);
		AssetManager::Remove(m_IFFTDYDXCOMBINEDYDZ.Ping);
		AssetManager::Remove(m_IFFTDYDXCOMBINEDYDZ.Pong);
		AssetManager::Remove(m_Spectrum);
		AssetManager::Remove(m_ConjSpectrum);
		AssetManager::Remove(m_Normal);
		AssetManager::Remove(m_Displacement);
	}

	WaterSystem::Cascade::Cascade(SpectrumData& data, CommonSpectrumData& commonData, const FFT& fft) :
		m_SpectrumData(data),
		m_CommonData(commonData),
		m_FFT(fft)
	{
		Init(commonData.N);
	}

	void WaterSystem::Run(EcsImpl::EntityManager& manager, GameContext& context)
	{
		for (auto& cascade : m_Cascades)
			cascade.CheckParametersChange();

		m_Data.DimensionChanged = false;

		for (auto& cascade : m_Cascades)
			cascade.Update(context.Time, context.DeltaTime);
	}

	void WaterSystem::Cascade::CheckParametersChange()
	{
		if (!m_SpectrumData.ParametersChanged && !m_CommonData.DimensionChanged)
			return;

		if (m_CommonData.DimensionChanged)
		{
			Cleanup();
			Init(m_CommonData.N);
		}

		auto [spectrum, g0] = AssetManager::GetAsset(m_SpectrumShader);
		auto [conjSpectrum, g1] = AssetManager::GetAsset(m_ConjSpectrumShader);

		auto [gaussianTex, g2] = AssetManager::GetAsset(m_Gaussian);
		auto [spectrumTex, g3] = AssetManager::GetAsset(m_Spectrum);
		auto [conjSpectrumTex, g4] = AssetManager::GetAsset(m_ConjSpectrum);

		if (!spectrum || !conjSpectrum)
		{
			spdlog::error("Water simulation shaders are not loaded.");
			return;
		}

		if (!gaussianTex || !spectrumTex || !conjSpectrumTex)
		{
			spdlog::error("Water simulation textures are not loaded.");
			return;
		}

		spectrum->Use();
		spectrum->SetUniform("uSize", m_CommonData.N);
		spectrum->SetUniform("uLengthScale", m_SpectrumData.L);

		spectrum->SetUniform("uWindSpeed", m_CommonData.WindSpeed);
		spectrum->SetUniform("uWindAngle", m_CommonData.WindAngleDegree);

		spectrum->SetUniform("uLowCutoff", m_SpectrumData.LowCutoff);
		spectrum->SetUniform("uHighCutoff", m_SpectrumData.HighCutoff);

		spectrum->SetUniform("uSwell", m_CommonData.Swell);
		spectrum->SetUniform("uFetch", m_CommonData.Fetch);
		spectrum->SetUniform("uDepth", m_CommonData.Depth);

		spectrum->BindReadTexture(*gaussianTex, 0);
		spectrum->BindWriteTexture(*spectrumTex, 1);
		spectrum->DispatchWait({ m_GroupCount, m_GroupCount }, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		conjSpectrum->Use();
		conjSpectrum->SetUniform("N", m_CommonData.N);
		conjSpectrum->SetUniform("L", m_SpectrumData.L);

		conjSpectrum->BindReadTexture(*spectrumTex, 0);
		conjSpectrum->BindWriteTexture(*conjSpectrumTex, 1);
		conjSpectrum->DispatchWait({ m_GroupCount, m_GroupCount }, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		m_SpectrumData.ParametersChanged = false;
	}

	void WaterSystem::Cascade::UpdateTimeSpectrum(f64 time)
	{
		auto [htPass, g0] = AssetManager::GetAsset(m_HtPassShader);

		auto [spectrumTex, g1] = AssetManager::GetAsset(m_Spectrum);
		auto [conjSpectrumTex, g2] = AssetManager::GetAsset(m_ConjSpectrum);

		auto [dyddxzTex, g3] = AssetManager::GetAsset(m_IFFTDYCOMBINEDDXZ.Ping);
		auto [dxdzTex, g4] = AssetManager::GetAsset(m_IFFTDXCOMBINEDZ.Ping);
		auto [ddxddzTex, g5] = AssetManager::GetAsset(m_IFFTDDXCOMBINEDDZ.Ping);
		auto [dydxdydzTex, g6] = AssetManager::GetAsset(m_IFFTDYDXCOMBINEDYDZ.Ping);

		if (!htPass)
		{
			spdlog::error("Water simulation shaders are not loaded.");
			return;
		}

		if (!spectrumTex || !conjSpectrumTex || !dyddxzTex || !dxdzTex || !ddxddzTex || !dydxdydzTex)
		{
			spdlog::error("Water simulation textures are not loaded.");
			return;
		}

		htPass->Use();
		htPass->SetUniform("t", m_SpectrumData.TimeScale * time);
		htPass->SetUniform("N", m_CommonData.N);
		htPass->SetUniform("L", m_SpectrumData.L);

		htPass->BindReadTexture(*spectrumTex, 0);
		htPass->BindReadTexture(*conjSpectrumTex, 1);

		htPass->BindWriteTexture(*dyddxzTex, 2);
		htPass->BindWriteTexture(*dxdzTex, 3);
		htPass->BindWriteTexture(*ddxddzTex, 4);
		htPass->BindWriteTexture(*dydxdydzTex, 5);

		htPass->DispatchWait({ m_GroupCount, m_GroupCount }, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	void WaterSystem::CombineResults(f32 dt)
	{
		for (auto& cascade : m_Cascades)
			cascade.CombineResults(dt);
	}

	void WaterSystem::Cascade::Update(f64 time, f32 dt)
	{
		UpdateTimeSpectrum(time);

		m_dycombineddxz = m_FFT.IFFT2D(m_IFFTDYCOMBINEDDXZ);
		m_dxcombinedz = m_FFT.IFFT2D(m_IFFTDXCOMBINEDZ);
		m_ddxcombineddz = m_FFT.IFFT2D(m_IFFTDDXCOMBINEDDZ);
		m_dydxcombinedydz = m_FFT.IFFT2D(m_IFFTDYDXCOMBINEDYDZ);

		CombineResults(dt);
	}

	void WaterSystem::Cascade::CombineResults(f32 dt)
	{
		auto [dyddxzTex, g0] = AssetManager::GetAsset(m_dycombineddxz);
		auto [dxdzTex, g1] = AssetManager::GetAsset(m_dxcombinedz);
		auto [ddxddzTex, g2] = AssetManager::GetAsset(m_ddxcombineddz);
		auto [dydxdydzTex, g3] = AssetManager::GetAsset(m_dydxcombinedydz);

		auto [dispTex, g4] = AssetManager::GetAsset(m_Displacement);
		auto [normalTex, g5] = AssetManager::GetAsset(m_Normal);

		auto [combineShader, g7] = AssetManager::GetAsset(m_CombineShader);
		auto [blurShader, g8] = AssetManager::GetAsset(m_BlurShader);

		if (!combineShader || !blurShader)
		{
			spdlog::error("Water simulation shaders are not loaded.");
			return;
		}

		if (!dispTex || !normalTex || !dyddxzTex || !dxdzTex || !ddxddzTex || !dydxdydzTex)
		{
			spdlog::error("Water simulation textures are not loaded.");
			return;
		}

		{
			combineShader->Use();
			combineShader->SetUniform("uLambda", m_SpectrumData.Lambda);
			combineShader->SetUniform("N", m_CommonData.N);
			combineShader->SetUniform("L", m_SpectrumData.L);
			combineShader->SetUniform("foamDecay", m_CommonData.FoamDecay);
			combineShader->SetUniform("foamIntensity", m_CommonData.FoamAdd);
			combineShader->SetUniform("dt", dt);

			combineShader->BindReadTexture(*dyddxzTex, 0);
			combineShader->BindReadTexture(*dxdzTex, 1);
			combineShader->BindReadTexture(*ddxddzTex, 2);
			combineShader->BindReadTexture(*dydxdydzTex, 3);

			combineShader->BindReadWriteTexture(*dispTex, 4);
			combineShader->BindReadWriteTexture(*normalTex, 5);

			combineShader->DispatchWait({ m_GroupCount, m_GroupCount }, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
		}
		{
			blurShader->Use();
			blurShader->SetUniform("N", m_CommonData.N);

			blurShader->BindReadWriteTexture(*normalTex, 0);

			blurShader->DispatchWait({ m_GroupCount, m_GroupCount }, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
		}

		dispTex->GenerateMips();
		normalTex->GenerateMips();
	}
}