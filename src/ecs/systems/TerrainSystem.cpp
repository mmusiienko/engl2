#include "ecs/systems/TerrainSystem.h"
#include "ui/Components.h"
#include "resources/StaticModel.h"
#include "Printer.h"

namespace EnGl::System
{
	struct TerrainSurface : public Material::Base
	{
		TerrainSystem& m_TerrainSystem;

		TerrainSurface(TerrainSystem& system) : m_TerrainSystem(system), Base(AssetManager::GRAPHICS_SHADER_DIR / "TerrainSurface") {}

		void SetCommonUniforms(Shader* shader, const GameContext& context) override
		{
			Base::SetCommonUniforms(shader, context);

			auto cam = context.Camera.Get();
			shader->SetUniform("uViewProjection", cam.ViewProjection);
			auto mainCam = context.Camera.GetTarget().Position;
			shader->SetUniform("uCameraPos", mainCam);
			shader->SetUniform("uDirectionalLight", context.DirLight.Data);
			shader->SetUniform("uPointLights", context.PointLights);
			shader->SetUniform("uResolution", context.Framebuffer.MainFramebuffer->Resolution());

			auto shadowMap = AssetManager::GetAsset(context.Renderer.CascadedShadowMap.ShadowMap).Asset;
			if (shadowMap)
			{
				shader->SetUniform("uShadowMap", *shadowMap, 0);
			}

			auto terrainInfo = AssetManager::GetAsset(m_TerrainSystem.m_Props.TerrainInfo.Texture).Asset;

			if (terrainInfo)
			{
				shader->SetUniform("uTerrainInfo", *terrainInfo, 1);
				shader->SetUniform("uScale", m_TerrainSystem.m_Props.Scale);
				shader->SetUniform("uHeightWeights", m_TerrainSystem.m_Props.HeightWeights);
				shader->SetUniform("uTerrainResolution", m_TerrainSystem.m_Props.Resolution);
			}
		}

		void SetCommonUniformsUnlit(Shader* shader, const GameContext& context) override
		{
			Base::SetCommonUniforms(shader, context);

			auto cam = context.Camera.Get();
			shader->SetUniform("uViewProjection", cam.ViewProjection);
			auto mainCam = context.Camera.GetTarget().Position;
			shader->SetUniform("uCameraPos", mainCam);

			auto terrainInfo = AssetManager::GetAsset(m_TerrainSystem.m_Props.TerrainInfo.Texture).Asset;

			if (terrainInfo)
			{
				shader->SetUniform("uTerrainInfo", *terrainInfo, 2);
				shader->SetUniform("uScale", m_TerrainSystem.m_Props.Scale);
				shader->SetUniform("uHeightWeights", m_TerrainSystem.m_Props.HeightWeights);
				shader->SetUniform("uTerrainResolution", m_TerrainSystem.m_Props.Resolution);
			}
		}
	};

	void TerrainSystem::Run(EcsImpl::EntityManager& manager, GameContext& context)
	{
		auto [transform, follow] = manager.Get<Component::Transform, Component::FollowSnap>(m_Surface);
		transform.Scale.x = context.Camera.Get().Far * 2.0f;
		transform.Scale.y = context.Camera.Get().Far * 2.0f;
		transform.Dirty = true;

		follow.PosOffset.z = -transform.Scale.y / 2.0f;
		follow.PosOffset.x = -transform.Scale.x / 2.0f;

		follow.Follow = context.Camera.GetEntity();

		m_Props.Resolution = transform.Scale.x / m_Props.TerrainResolution;

		follow.Snap = m_Props.Resolution;
	}


	void TerrainSystem::Init(EcsImpl::EntityManager& manager)
	{
		m_Surface = manager.Create<
			Component::Transform, Component::ModelMatrix, Component::RenderedModel, Component::FollowSnap
		>(
			[=](Component::Transform& transform, auto&, Component::RenderedModel& model, Component::FollowSnap& follow) -> void
			{
				auto mat = make_scope<TerrainSurface>(*this);
				mat->SetUnlit(AssetManager::GRAPHICS_SHADER_DIR / "TerrainSurface");

				auto matH = AssetManager::PutScope<Material::Base>(std::move(mat));
				model.Model = StaticModel::QuadTesselated(matH, m_Props.TerrainResolution, m_Props.TerrainResolution);
				model.Layer = Component::RenderLayer::OQ;
				transform.Rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
				follow.PosUnlock = { true, false, true };
			}, "TerrainSurface"
		);

		m_Props.TerrainInfo.Fill();
	}

	void TerrainSystem::Editor(EcsImpl::EntityManager& manager, GameContext& context)
	{
		ImGui::Text("Terrain System");

		UiComponents::Noise2DView(m_Props.TerrainInfo);
		ImGui::InputFloat4("HeightWeights", glm::value_ptr(m_Props.HeightWeights));
		ImGui::InputFloat4("Scale", glm::value_ptr(m_Props.Scale));
	}
}