#include "CloudSystem.h"
#include "../ui/Components.h"
#include "../resources/StaticModel.h"


namespace EnGl
{
	const std::vector<glm::vec2> CloudSystem::CrossOffset4x4 
	{
			glm::vec2{0,0},
			glm::vec2{2,2},
			glm::vec2{2,0},
			glm::vec2{0,2},
			glm::vec2{1,1},
			glm::vec2{3,3},
			glm::vec2{3,1},
			glm::vec2{1,3},
			glm::vec2{1,0},
			glm::vec2{3,2},
			glm::vec2{3,0},
			glm::vec2{1,2},
			glm::vec2{0,1},
			glm::vec2{2,3},
			glm::vec2{2,1},
			glm::vec2{0,3}
	};

	void CloudSystem::Init(EcsImpl::EntityManager& manager)
	{
		m_SkyTexture = AssetManager::Put<Texture2D>(1u, 1u, Texture::CreationInfoFromData{ 
			.CpuFormat = GL_RGBA,
			.GpuFormat = GL_RGBA32F,
			.Common = {.MinFilter = GL_NEAREST, .MagFilter = GL_NEAREST}
		});
		m_SkyDepth = AssetManager::Put<Texture2D>(1u, 1u, Texture::CreationInfoFromData{
			.CpuFormat = GL_RED,
			.GpuFormat = GL_R32F,
			.Common = {.MinFilter = GL_NEAREST, .MagFilter = GL_NEAREST}
		});
		m_TransmittanceLUT = AssetManager::Put<Texture2D>(
			static_cast<u32>(m_AtmosphereParams.TransmittanceLUTRes.x),
			static_cast<u32>(m_AtmosphereParams.TransmittanceLUTRes.y),
		
			Texture::CreationInfoFromData{
				.CpuFormat = GL_RED,
				.GpuFormat = GL_R32F,
				.Common = {.MinFilter = GL_NEAREST, .MagFilter = GL_NEAREST}
		});

		
		m_Params.Shape.Fill();
		m_Params.Detail.Fill();
		m_Params.Weather.Fill();

		PrecomputeAtmosphere();
	}

	void CloudSystem::PrecomputeAtmosphere()
	{
		PrecomputeTransmittance();

		ComputeShader::Wait(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	void CloudSystem::PrecomputeTransmittance()
	{
		auto tex = AssetManager::GetAsset(m_TransmittanceLUT).Asset;

		if (!tex)
		{
			spdlog::error("Transmittance lut texture is not loaded.");
			return;
		}

		auto shader = AssetManager::GetAsset(m_TransmittanceLUTShader).Asset;
		if (!shader)
		{
			spdlog::error("Shader for transmittance lut precomputation is not loaded.");
			return;
		}

		shader->Use();
		shader->BindWriteTexture(*tex, 0);
		shader->Dispatch(
			{
				.GroupSizeX = static_cast<u32> (glm::ceil(m_AtmosphereParams.TransmittanceLUTRes.x / 16.0f)),
				.GroupSizeY = static_cast<u32> (glm::ceil(m_AtmosphereParams.TransmittanceLUTRes.y / 16.0f))
			}
		);
	}

	void CloudSystem::Run(EcsImpl::EntityManager& manager, GameContext& context)
	{
		auto tex = AssetManager::GetAsset(m_SkyTexture).Asset;
		auto depthTex = AssetManager::GetAsset(m_SkyDepth).Asset;

		if (!tex || !depthTex)
		{
			spdlog::error("Texture for sky is not loaded.");
			return;
		}

		if (context.Framebuffer.MainFramebuffer->Resolution() != m_Res)
		{
			m_Res = context.Framebuffer.MainFramebuffer->Resolution();
			tex->Properties().w = m_Res.x;
			tex->Properties().h = m_Res.y;
			tex->Update();

			depthTex->Properties().w = m_Res.x;
			depthTex->Properties().h = m_Res.y;
			depthTex->Update();
		}

		auto [shader, g] = AssetManager::GetAsset(m_Shader);
		if (!shader)
		{
			spdlog::error("Shader for sky is not loaded.");
			return;
		}

		shader->Use();
		shader->BindWriteTexture(*tex, 0);
		shader->BindWriteTexture(*depthTex, 1);
		auto cam = context.Camera.Get();

		shader->SetUniform("uCurrentOffset", m_CurrentRenderOffset[m_CurrentRenderOffsetIdx]);

		m_CurrentRenderOffsetIdx++;
		m_CurrentRenderOffsetIdx %= m_CurrentRenderOffset.size();

		shader->SetUniform("uInvProjection", cam.InverseProjection);
		shader->SetUniform("uInvView", cam.InverseView);
		shader->SetUniform("uCameraPos", *cam.Position);
		shader->SetUniform("uNear", cam.Near);
		shader->SetUniform("uFar", cam.Far);
		shader->SetUniform("uResolution", context.Framebuffer.MainFramebuffer->Resolution());
		shader->SetUniform("uDirectionalLight", context.DirLight);
		shader->SetUniform("uTime", static_cast<f32>(context.Time));

		auto depth = AssetManager::GetAsset(context.Framebuffer.MainFramebuffer->Depth()).Asset;
		auto color = AssetManager::GetAsset(context.Framebuffer.MainFramebuffer->Color()[0]).Asset;
		if (depth && color)
		{
			shader->SetUniform("uDepth", *depth, 2);
		}

		auto tex1 = AssetManager::GetAsset(m_Params.Shape.Texture).Asset;
		auto tex2 = AssetManager::GetAsset(m_Params.Detail.Texture).Asset;
		auto tex3 = AssetManager::GetAsset(m_Params.Weather.Texture).Asset;
		if (!tex1 || !tex2 || !tex3)
		{
			spdlog::error("Shape textures for sky are not loaded.");
			return;
		}

		shader->SetUniform("uShape", *tex1, 3);
		shader->SetUniform("uDetail", *tex2, 4);
		shader->SetUniform("uWeather", *tex3, 5);
		shader->SetUniform("uResScale", m_Params.ResScale);
		shader->SetUniform("uCloudScale", m_Params.CloudScale);
		shader->SetUniform("uDetailScale", m_Params.DetailScale);
		shader->SetUniform("uWeatherScale", m_Params.WeatherMapScale);
		shader->SetUniform("uGlobalCoverage", m_Params.GlobalCoverage);
		shader->SetUniform("uGlobalOpacity", m_Params.GlobalOpacity);

		shader->SetUniform("uNumSteps", m_Params.NumRaySteps);
		shader->SetUniform("uNumStepsLight", m_Params.NumRayStepsLight);

		shader->SetUniform("uSkySpan", m_Params.SkySpan);
		shader->SetUniform("uSkyHeightMin", m_Params.SkyHeightMin);
		shader->SetUniform("uSkyHeightMax", m_Params.SkyHeightMax);

		shader->SetUniform("uSpeed", m_Params.Speed);
		shader->SetUniform("uDirection", m_Params.Direction);

		shader->SetUniform("uDarknessThreshold", m_Params.DarknessThreshold);
		shader->SetUniform("uLightAbsorptionSun", m_Params.LightAbsorptionSun);
		shader->SetUniform("uLightAbsorptionCloud", m_Params.LightAbsorptionCloud);
		shader->SetUniform("uPhaseVal", m_Params.PhaseVal);

		shader->SetUniform("uAnvil", m_Params.Anvil);
		shader->SetUniform("uCubemap", *context.Cubemap, 6);

		shader->DispatchWait(
			{
				.GroupSizeX = static_cast<u32> (glm::ceil(m_Res.x / m_Params.ResScale / 16.0f)),
				.GroupSizeY = static_cast<u32> (glm::ceil(m_Res.y / m_Params.ResScale / 16.0f))
			}
			, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
		);
		tex->GenerateMips();
		context.SkyTexture = m_SkyTexture;
		context.SkyDepthTexture = m_SkyDepth;
	}

	void CloudSystem::Editor(EcsImpl::EntityManager& manager, GameContext& context)
	{
		ImGui::SeparatorText("Cloud system");
		UiComponents::Texture2DView(m_TransmittanceLUT);

		ImGui::PushID(0);
		UiComponents::Noise3DView(m_Params.Shape);
		ImGui::PopID();
		ImGui::PushID(1);
		UiComponents::Noise3DView(m_Params.Detail);
		ImGui::PopID();
		UiComponents::Noise2DView(m_Params.Weather);
		ImGui::SliderFloat("GlobalCoverage", &m_Params.GlobalCoverage, 0.0f, 1.0f);
		ImGui::SliderFloat("GlobalOpacity", &m_Params.GlobalOpacity, 0.0f, 1.0f);
		ImGui::SliderFloat("CloudScale", &m_Params.CloudScale, 0.001f, 10.0f);
		ImGui::SliderFloat("DetailScale", &m_Params.DetailScale, 0.001f, 10.0f);
		ImGui::SliderFloat("WeatherScale", &m_Params.WeatherMapScale, 0.001f, 10.0f);

		UiComponents::InputUInt("NumSteps", &m_Params.NumRaySteps);
		UiComponents::InputUInt("NumStepsLight", &m_Params.NumRayStepsLight);

		ImGui::InputFloat("SkySpan", &m_Params.SkySpan);
		ImGui::InputFloat("SkyHeightMin", &m_Params.SkyHeightMin);
		ImGui::InputFloat("SkyHeightMax", &m_Params.SkyHeightMax);

		ImGui::InputFloat("Speed", &m_Params.Speed);
		ImGui::InputFloat3("Direction", glm::value_ptr(m_Params.Direction));

		ImGui::InputFloat("DarknessThreshold", &m_Params.DarknessThreshold);
		ImGui::InputFloat("LightAbsorptionSun", &m_Params.LightAbsorptionSun);
		ImGui::InputFloat("LightAbsorptionCloud", &m_Params.LightAbsorptionCloud);
		ImGui::InputFloat("PhaseVal", &m_Params.PhaseVal);

		ImGui::SliderFloat("Anvil", &m_Params.Anvil, 0.0f, 1.0f);
	}
}
