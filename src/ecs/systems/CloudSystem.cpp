#include "CloudSystem.h"

#include "ui/Components.h"


namespace EnGl
{
	void CloudSystem::Init(EcsImpl::EntityManager& manager)
	{
		m_SkyTextureLowResA = AssetManager::Put<Texture2D>(1u, 1u, Texture::CreationInfoFromData{
			.CpuFormat = GL_RGBA,
			.GpuFormat = GL_RGBA32F,
			.Common = {.MinFilter = GL_LINEAR, .MagFilter = GL_LINEAR}
		});

		m_SkyTextureLowResB = AssetManager::Put<Texture2D>(1u, 1u, Texture::CreationInfoFromData{
			.CpuFormat = GL_RGBA,
			.GpuFormat = GL_RGBA32F,
			.Common = {.MinFilter = GL_LINEAR, .MagFilter = GL_LINEAR}
		});

		m_SkyTexture = AssetManager::Put<Texture2D>(1u, 1u, Texture::CreationInfoFromData{ 
			.CpuFormat = GL_RGBA,
			.GpuFormat = GL_RGBA32F,
			.Common = {.Wrap = GL_CLAMP_TO_EDGE, .MinFilter = GL_LINEAR, .MagFilter = GL_LINEAR}
		});

		m_HistoryTexture = AssetManager::Put<Texture2D>(1u, 1u, Texture::CreationInfoFromData{
			.CpuFormat = GL_RGBA,
			.GpuFormat = GL_RGBA32F,
			.Common = {.Wrap = GL_CLAMP_TO_EDGE, .MinFilter = GL_LINEAR, .MagFilter = GL_LINEAR}
		});

		m_TransmittanceLUT = AssetManager::Put<Texture2D>(
			static_cast<u32>(m_AtmosphereParams.TransmittanceLUTRes.x),
			static_cast<u32>(m_AtmosphereParams.TransmittanceLUTRes.y),
		
			Texture::CreationInfoFromData{
				.CpuFormat = GL_RED,
				.GpuFormat = GL_R32F,
				.Common = {.MinFilter = GL_LINEAR, .MagFilter = GL_LINEAR}
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

	CloudSystem::CloudTextures CloudSystem::GetTextures(GameContext& context)
	{
		auto texLowResA = AssetManager::GetAsset(m_SkyTextureLowResA).Asset;
		auto tex = AssetManager::GetAsset(m_SkyTexture).Asset;
		auto history = AssetManager::GetAsset(m_HistoryTexture).Asset;
		auto texLowResB = AssetManager::GetAsset(m_SkyTextureLowResB).Asset;

		if (context.Framebuffer.MainFramebuffer->Resolution() != m_Res)
		{
			m_Res = context.Framebuffer.MainFramebuffer->Resolution();
			texLowResA->Properties().w = m_Res.x / m_Params.ResScale;
			texLowResA->Properties().h = m_Res.y / m_Params.ResScale;
			texLowResA->Update();

			texLowResB->Properties().w = m_Res.x / m_Params.ResScale;
			texLowResB->Properties().h = m_Res.y / m_Params.ResScale;
			texLowResB->Update();

			tex->Properties().w = m_Res.x;
			tex->Properties().h = m_Res.y;
			tex->Update();

			history->Properties().w = m_Res.x;
			history->Properties().h = m_Res.y;
			history->Update();
		}

		return { .LowResA = texLowResA, .LowResB = texLowResB  , .Sky = tex, .History = history };
	}

	void CloudSystem::Run(EcsImpl::EntityManager& manager, GameContext& context)
	{
		std::swap(m_SkyTexture, m_HistoryTexture);

		auto textures = GetTextures(context);
		
		if (!textures.History || !textures.LowResA || !textures.LowResB || !textures.Sky) return;

		m_CurrentRenderOffsetIdx++;
		m_CurrentRenderOffsetIdx %= static_cast<u32>(m_Params.ResScale * m_Params.ResScale);

		DispatchLowRes(context, textures);

		DispatchUpsample(context, textures);

		//DispatchBlur(context, textures);

		//std::swap(m_SkyTexture, m_HistoryTexture);

		DispatchEdge(context, textures);

		DispatchFullRes(context, textures);

		//std::swap(m_SkyTexture, m_HistoryTexture);

		//DispatchBlur(context, textures);

		context.SkyTexture = m_SkyTexture;
		context.SkyTextureLowRes = m_SkyTextureLowResA;
	}

	void CloudSystem::DispatchLowRes(GameContext& context, CloudTextures& textures)
	{
		auto shader = AssetManager::GetAsset(m_LowResShader).Asset;
		auto tex1 = AssetManager::GetAsset(m_Params.Shape.Texture).Asset;
		auto tex2 = AssetManager::GetAsset(m_Params.Detail.Texture).Asset;
		auto tex3 = AssetManager::GetAsset(m_Params.Weather.Texture).Asset;

		if (!tex1 || !tex2 || !tex3)
		{
			spdlog::error("Shape textures for sky are not loaded.");
			return;
		}

		if (!shader)
		{
			spdlog::error("Shader for sky is not loaded.");
			return;
		}

		auto& cam = context.Camera.Get();

		shader->Use();
		shader->BindWriteTexture(*textures.LowResA, 0);
		shader->BindWriteTexture(*textures.LowResB, 1);

		shader->SetUniform("uCurrentOffset", m_CurrentRenderOffsetIdx);

		shader->SetUniform("uInvViewProjection", cam.InverseViewProjection);
		shader->SetUniform("uCameraPos", *cam.Position);
		shader->SetUniform("uCameraDelta", cam.Delta);
		shader->SetUniform("uCameraForward", cam.Forward);
		shader->SetUniform("uNear", cam.Near);
		shader->SetUniform("uFar", cam.Far);
		shader->SetUniform("uResolution", m_Res);
		shader->SetUniform("uDirectionalLight", context.DirLight.Data);
		shader->SetUniform("uTime", static_cast<f32>(context.Time));

		auto depth = AssetManager::GetAsset(context.Framebuffer.MainFramebuffer->Depth()).Asset;
		if (depth)
		{
			shader->SetUniform("uDepth", *depth, 2);
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

		shader->SetUniform("uSkyHeightMin", m_Params.SkyHeightMin);
		shader->SetUniform("uSkyHeightMax", m_Params.SkyHeightMax);
		shader->SetUniform("uPlanetRadius", m_Params.PlanetRadius);

		shader->SetUniform("uSpeed", m_Params.Speed);
		shader->SetUniform("uDirection", m_Params.Direction);

		shader->SetUniform("uDarknessThreshold", m_Params.DarknessThreshold);
		shader->SetUniform("uLightAbsorptionSun", m_Params.LightAbsorptionSun);
		shader->SetUniform("uLightAbsorptionCloud", m_Params.LightAbsorptionCloud);
		shader->SetUniform("uPhaseVal", m_Params.PhaseVal);

		shader->SetUniform("uAnvil", m_Params.Anvil);
		shader->SetUniform("uCubemap", *context.Cubemap, 6);

		shader->SetUniform("uShapeBottomCut", m_Params.ShapeBottomCut);
		shader->SetUniform("uDensityBottomCut", m_Params.DensityBottomCut);
		shader->SetUniform("uDensityTopCut", m_Params.DensityTopCut);

		shader->SetUniform("uBeer", m_Params.Beer);
		shader->SetUniform("uInScatter", m_Params.InScatter);
		shader->SetUniform("uOutScatter", m_Params.OutScatter);
		shader->SetUniform("uInOutCoefficient", m_Params.InOutCoefficient);
		shader->SetUniform("uSilverLine", m_Params.SilverLine);
		shader->SetUniform("uSilverLineExp", m_Params.SilverLineExp);
		shader->SetUniform("uAmbient", m_Params.Ambient);

		shader->SetUniform("uExtinctionColor", m_Params.ExtinctionColor);

		shader->DispatchWait(
			{
				.GroupSizeX = static_cast<u32> (glm::ceil(m_Res.x / m_Params.ResScale / 16.0f)),
				.GroupSizeY = static_cast<u32> (glm::ceil(m_Res.y / m_Params.ResScale / 16.0f))
			}
			, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
		);
	}

	void CloudSystem::DispatchUpsample(GameContext& context, CloudTextures& textures)
	{
		auto shader = AssetManager::GetAsset(m_UpsampleShader).Asset;

		if (!shader)
		{
			spdlog::error("Shader for sky is not loaded.");
			return;
		}
		
		auto& cam = context.Camera.Get();

		shader->Use();
		shader->BindWriteTexture(*textures.Sky, 0);

		shader->SetUniform("uLowResA", *textures.LowResA, 1);
		shader->SetUniform("uLowResB", *textures.LowResB, 2);
		shader->SetUniform("uHistory", *textures.History, 3);

		shader->SetUniform("uNear", cam.Near);
		shader->SetUniform("uFar", cam.Far);

		auto depth = AssetManager::GetAsset(context.Framebuffer.MainFramebuffer->Depth()).Asset;
		if (depth)
		{
			shader->SetUniform("uDepth", *depth, 4);
		}
		shader->SetUniform("uCameraForward", cam.Forward);
		shader->SetUniform("uCurrentOffset", m_CurrentRenderOffsetIdx);
		shader->SetUniform("uCameraPos", *cam.Position);
		shader->SetUniform("uInvViewProjection", cam.InverseViewProjection);
		shader->SetUniform("uViewProjectionLastFrame", cam.ViewProjectionLastFrame);
		shader->SetUniform("uResolution", m_Res);
		shader->SetUniform("uResScale", m_Params.ResScale);
		shader->SetUniform("uViewProjectionLastFrame", cam.ViewProjectionLastFrame);


		shader->DispatchWait(
			{
				.GroupSizeX = static_cast<u32> (glm::ceil(m_Res.x / 16.0f)),
				.GroupSizeY = static_cast<u32> (glm::ceil(m_Res.y / 16.0f))
			}
			, GL_TEXTURE_FETCH_BARRIER_BIT | GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
		);
	}

	void CloudSystem::DispatchDepthNeighborFill(GameContext& context, CloudTextures& textures)
	{
		auto shader = AssetManager::GetAsset(m_DepthNeighborShader).Asset;

		if (!shader)
		{
			spdlog::error("Shader for sky is not loaded.");
			return;
		}

		auto depth = AssetManager::GetAsset(context.Framebuffer.MainFramebuffer->Depth()).Asset;
		if (!depth)
			return;

		auto& cam = context.Camera.Get();

		shader->Use();

		shader->BindWriteTexture(*textures.LowResB, 0);
		shader->SetUniform("uDepth", *depth, 1);
		shader->SetUniform("uResScale", m_Params.ResScale);
		shader->SetUniform("uResolution", m_Res);
		shader->SetUniform("uNear", cam.Near);
		shader->SetUniform("uFar", cam.Far);

		shader->DispatchWait(
			{
				.GroupSizeX = static_cast<u32> (glm::ceil(m_Res.x / m_Params.ResScale / 16.0f)),
				.GroupSizeY = static_cast<u32> (glm::ceil(m_Res.y / m_Params.ResScale / 16.0f))
			}
			, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
		);
	}

	void CloudSystem::DispatchBlur(GameContext& context, CloudTextures& textures)
	{
		auto shader = AssetManager::GetAsset(m_BlurShader).Asset;

		if (!shader)
		{
			spdlog::error("Shader for sky is not loaded.");
			return;
		}

		shader->Use();

		shader->BindReadTexture(*textures.Sky, 0);
		shader->BindWriteTexture(*textures.History, 1);

		shader->DispatchWait(
			{
				.GroupSizeX = static_cast<u32> (glm::ceil(m_Res.x / 16.0f)),
				.GroupSizeY = static_cast<u32> (glm::ceil(m_Res.y / 16.0f))
			}
			, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
		);
	}

	void CloudSystem::DispatchEdge(GameContext& context, CloudTextures& textures)
	{
		auto shader = AssetManager::GetAsset(m_EdgeShader).Asset;

		if (!shader)
		{
			spdlog::error("Shader for sky is not loaded.");
			return;
		}

		auto depth = AssetManager::GetAsset(context.Framebuffer.MainFramebuffer->Depth()).Asset;
		if (!depth)
			return;

		auto& cam = context.Camera.Get();

		shader->Use();

		shader->BindReadTexture(*textures.Sky, 0);
		shader->BindWriteTexture(*textures.History, 1);
		shader->SetUniform("uDepth", *depth, 2);
		shader->SetUniform("uResScale", m_Params.ResScale);
		shader->SetUniform("uResolution", m_Res);
		shader->SetUniform("uNear", cam.Near);
		shader->SetUniform("uFar", cam.Far);

		shader->DispatchWait(
			{
				.GroupSizeX = static_cast<u32> (glm::ceil(m_Res.x / 16.0f)),
				.GroupSizeY = static_cast<u32> (glm::ceil(m_Res.y / 16.0f))
			}
			, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
		);
	}

	void CloudSystem::DispatchFullRes(GameContext & context, CloudTextures & textures)
	{
		auto shader = AssetManager::GetAsset(m_FullResShader).Asset;
		auto tex1 = AssetManager::GetAsset(m_Params.Shape.Texture).Asset;
		auto tex2 = AssetManager::GetAsset(m_Params.Detail.Texture).Asset;
		auto tex3 = AssetManager::GetAsset(m_Params.Weather.Texture).Asset;

		if (!tex1 || !tex2 || !tex3)
		{
			spdlog::error("Shape textures for sky are not loaded.");
			return;
		}

		if (!shader)
		{
			spdlog::error("Shader for sky is not loaded.");
			return;
		}

		auto& cam = context.Camera.Get();

		shader->Use();
		shader->BindReadTexture(*textures.History, 0);
		shader->BindWriteTexture(*textures.Sky, 1);

		shader->SetUniform("uCurrentOffset", m_CurrentRenderOffsetIdx);

		shader->SetUniform("uInvViewProjection", cam.InverseViewProjection);
		shader->SetUniform("uCameraPos", *cam.Position);
		shader->SetUniform("uCameraDelta", cam.Delta);
		shader->SetUniform("uCameraForward", cam.Forward);
		shader->SetUniform("uNear", cam.Near);
		shader->SetUniform("uFar", cam.Far);
		shader->SetUniform("uResolution", m_Res);
		shader->SetUniform("uDirectionalLight", context.DirLight.Data);
		shader->SetUniform("uTime", static_cast<f32>(context.Time));
		shader->SetUniform("uPlanetRadius", m_Params.PlanetRadius);

		auto depth = AssetManager::GetAsset(context.Framebuffer.MainFramebuffer->Depth()).Asset;
		if (depth)
		{
			shader->SetUniform("uDepth", *depth, 2);
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

		shader->SetUniform("uShapeBottomCut", m_Params.ShapeBottomCut);
		shader->SetUniform("uDensityBottomCut", m_Params.DensityBottomCut);
		shader->SetUniform("uDensityTopCut", m_Params.DensityTopCut);

		shader->SetUniform("uBeer", m_Params.Beer);
		shader->SetUniform("uInScatter", m_Params.InScatter);
		shader->SetUniform("uOutScatter", m_Params.OutScatter);
		shader->SetUniform("uInOutCoefficient", m_Params.InOutCoefficient);
		shader->SetUniform("uSilverLine", m_Params.SilverLine);
		shader->SetUniform("uSilverLineExp", m_Params.SilverLineExp);
		shader->SetUniform("uAmbient", m_Params.Ambient);

		shader->SetUniform("uExtinctionColor", m_Params.ExtinctionColor);

		shader->DispatchWait(
			{
				.GroupSizeX = static_cast<u32> (glm::ceil(m_Res.x / 16.0f)),
				.GroupSizeY = static_cast<u32> (glm::ceil(m_Res.y / 16.0f))
			}
			, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
		);
	}

	void CloudSystem::Editor(EcsImpl::EntityManager& manager, GameContext& context)
	{
		ImGui::SeparatorText("Cloud system");

		PresetSelect(manager, context);

		auto& dirLight = manager.Get<Component::DirectionalLight>(context.DirLight.Id);
		auto& cam = manager.Get<Component::PerspectiveProjection>(context.Camera.Get().Entity);

		ImGui::SeparatorText("Skybox shape");
		ImGui::InputFloat("SkyHeightMin", &m_Params.SkyHeightMin);
		ImGui::InputFloat("SkyHeightMax", &m_Params.SkyHeightMax);
		ImGui::SliderFloat("PlanetRadius", &m_Params.PlanetRadius, 400000.0f, 6000000.0f);
		cam.Dirty |= ImGui::InputFloat("CameraFar", &cam.FarPlane);

		ImGui::SeparatorText("Color");
		ImGui::SliderFloat("Ambient", &m_Params.Ambient, 0.0f, 1.0f);
		ImGui::ColorEdit3("DirLightColor", glm::value_ptr(dirLight.Color));
		ImGui::ColorEdit3("Extinction", glm::value_ptr(m_Params.ExtinctionColor));

		ImGui::InputFloat("Anvil", &m_Params.Anvil, 0.0f, 1.0f);
		ImGui::SliderFloat("GlobalCoverage", &m_Params.GlobalCoverage, 0.0f, 1.0f);
		ImGui::SliderFloat("GlobalOpacity", &m_Params.GlobalOpacity, 0.0f, 1.0f);
		ImGui::SliderFloat("CloudScale", &m_Params.CloudScale, 0.00001f, 1.0f);
		ImGui::SliderFloat("DetailScale", &m_Params.DetailScale, 0.00001f, 1.0f);
		ImGui::SliderFloat("WeatherScale", &m_Params.WeatherMapScale, 0.00001f, 3.0f);

		UiComponents::InputUInt("NumSteps", &m_Params.NumRaySteps);
		UiComponents::InputUInt("NumStepsLight", &m_Params.NumRayStepsLight);
		
		ImGui::InputFloat("Speed", &m_Params.Speed);
		ImGui::InputFloat3("Direction", glm::value_ptr(m_Params.Direction));

		ImGui::SliderFloat("Beer", &m_Params.Beer, 0.0f, 10.0f);
		ImGui::SliderFloat("InScatter", &m_Params.InScatter, 0.0f, 1.0f);
		ImGui::SliderFloat("OutScatter", &m_Params.OutScatter, 0.0f, 1.0f);
		ImGui::SliderFloat("InOutCoefficient", &m_Params.InOutCoefficient, 0.0f, 1.0f);
		ImGui::SliderFloat("SilverLineIntensity", &m_Params.SilverLine, 0.0f, 20.0f);
		ImGui::SliderFloat("SilverLineExp", &m_Params.SilverLineExp, 0.0f, 20.0f);

		ImGui::PushID(0);
		UiComponents::Noise3DView(m_Params.Shape);
		ImGui::PopID();
		ImGui::PushID(1);
		UiComponents::Noise3DView(m_Params.Detail);
		ImGui::PopID();
		UiComponents::Noise2DView(m_Params.Weather);

		UiComponents::Texture2DView(m_TransmittanceLUT);
	}

	void CloudSystem::PresetSelect(EcsImpl::EntityManager& manager, GameContext& context)
	{
		auto& dirLight = manager.Get<Component::DirectionalLight>(context.DirLight.Id);
		auto& cam = manager.Get<Component::PerspectiveProjection>(context.Camera.Get().Entity);

		for (auto& [presetName, preset] : m_Presets)
		{
			if (ImGui::Selectable(presetName.c_str()))
			{
				m_Params = preset.CloudParams;
				dirLight.Color = preset.DirLightColor;
				cam.FarPlane = preset.CameraFar;
				cam.Dirty = true;

				m_Params.Shape.Fill();
				m_Params.Detail.Fill();
				m_Params.Weather.Fill();
			}
		}
	}
}
