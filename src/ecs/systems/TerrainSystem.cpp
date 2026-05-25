#include "ecs/systems/TerrainSystem.h"
#include "ui/Components.h"
#include "resources/StaticModel.h"
#include "Printer.h"

namespace EnGl
{
	struct TerrainSurface : public Material::Base
	{
		TerrainSystem& m_TerrainSystem;

		TerrainSurface(TerrainSystem& system) : m_TerrainSystem(system), Base(AssetManager::GRAPHICS_SHADER_DIR / "TerrainSurface") {}

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
			m_Shader->SetUniform("uTime", static_cast<f32>(context.Time));
			m_Shader->SetUniform("uDirectionalLight", context.DirLight.Data);
			m_Shader->SetUniform("uPointLights", context.PointLights);

			auto cubemap = AssetManager::GetAsset(context.Cubemap.Asset).Asset;

			if (cubemap)
			{
				m_Shader->SetUniform("uCubemap", *cubemap, 0);
			}

			//auto shadowMap = AssetManager::GetAsset(context.Framebuffer.DirShadowFramebuffer->Depth()).Asset;
			//if (shadowMap)
			//{
			//	m_Shader->SetUniform("uShadowMap", *shadowMap, 1);
			//	m_Shader->SetUniform("uShadowMapViewProjection", context.Camera.GetDirShadowCamera().ViewProjection);
			//}

			auto terrainInfo = AssetManager::GetAsset(m_TerrainSystem.m_Props.TerrainInfo.Texture).Asset;

			if (terrainInfo)
			{
				m_Shader->SetUniform("uTerrainInfo", *terrainInfo, 2);
				m_Shader->SetUniform("uScale", m_TerrainSystem.m_Props.Scale);
				m_Shader->SetUniform("uHeightWeights", m_TerrainSystem.m_Props.HeightWeights);
				m_Shader->SetUniform("uResolution", m_TerrainSystem.m_Props.Resolution);
			}

			return ok;
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

		follow.Follow = context.Camera.Get().Entity;

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
				auto mat = AssetManager::PutScope<Material::Base>(make_scope<TerrainSurface>(*this));
				model.Model = StaticModel::QuadTesselated(mat, m_Props.TerrainResolution, m_Props.TerrainResolution);
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