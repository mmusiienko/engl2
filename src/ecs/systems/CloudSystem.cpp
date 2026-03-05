#include "CloudSystem.h"
#include "../ui/Components.h"
#include "../resources/StaticModel.h"


namespace EnGl
{
	void CloudSystem::Init(EcsImpl::EntityManager& manager)
	{
		m_SkyTexture = AssetManager::Put<Texture2D>(1u, 1u, Texture::CreationInfoFromData{ 
			.CpuFormat = GL_RGBA,
			.GpuFormat = GL_RGBA32F,
			.Common = {.MinFilter = GL_LINEAR_MIPMAP_LINEAR, .MagFilter = GL_LINEAR} 
		});
	}

	void CloudSystem::Run(EcsImpl::EntityManager& manager, GameContext& context)
	{
		auto [tex, g0] = AssetManager::GetAsset(m_SkyTexture);

		if (!tex)
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
		}

		auto [shader, g] = AssetManager::GetAsset(m_Shader);
		if (!shader)
		{
			spdlog::error("Shader for sky is not loaded.");
			return;
		}

		shader->Use();
		shader->BindWriteTexture(*tex, 0);
		auto cam = context.Camera.Get();
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
			shader->SetUniform("uDepth", *depth, 3);
		}

		auto [tex1, g3] = AssetManager::GetAsset(m_Params.Voronoi1FBM.VoronoiFBM);
		auto [tex2, g4] = AssetManager::GetAsset(m_Params.Voronoi2FBM.VoronoiFBM);
		if (tex1 && tex2)
		{
			shader->SetUniform("uWorley1", *tex1, 4);
			shader->SetUniform("uWorley2", *tex2, 5);
		}

		shader->SetUniform("uResScale", m_Params.ResScale);
		shader->SetUniform("uCloudScale", m_Params.CloudScale);
		shader->SetUniform("uDetailScale", m_Params.DetailScale);
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
		shader->DispatchWait(
			{
				.GroupSizeX = static_cast<u32> (glm::ceil(m_Res.x / m_Params.ResScale / 16.0f)),
				.GroupSizeY = static_cast<u32> (glm::ceil(m_Res.y / m_Params.ResScale / 16.0f))
			}
			, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
		);
		tex->GenerateMips();
		context.SkyTexture = m_SkyTexture;
	}

	void CloudSystem::Editor(EcsImpl::EntityManager& manager, GameContext& context)
	{
		ImGui::SeparatorText("Cloud system");
		ImGui::PushID(0);
		UiComponents::VoronoiNoiseView(m_Params.Voronoi1FBM);
		ImGui::PopID();
		ImGui::PushID(1);
		UiComponents::VoronoiNoiseView(m_Params.Voronoi2FBM);
		ImGui::PopID();
		ImGui::SliderFloat("GlobalCoverage", &m_Params.GlobalCoverage, 0.0f, 1.0f);
		ImGui::SliderFloat("GlobalOpacity", &m_Params.GlobalOpacity, 0.0f, 1.0f);
		ImGui::InputFloat3("CloudScale", glm::value_ptr(m_Params.CloudScale), "%.5f");
		ImGui::InputFloat3("DetailScale", glm::value_ptr(m_Params.DetailScale));

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
	}
}
