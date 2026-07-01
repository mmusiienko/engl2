#pragma once
#include "ecs/systems/Systems.h"


namespace EnGl::System
{
	class CascadedShadowMapFit : public SystemImpl
	{
		void Run(EcsImpl::EntityManager& manager, GameContext& context) override;
		void Editor(EcsImpl::EntityManager& manager, GameContext& context) override;
	};

	class CascadedShadowMapRender : public SystemImpl
	{
		void Init(EcsImpl::EntityManager& manager) override;
		void Run(EcsImpl::EntityManager& manager, GameContext& context) override;
		void Editor(EcsImpl::EntityManager& manager, GameContext& context) override;

		void RenderShadows(EcsImpl::EntityManager& manager, GameContext& context);
		void DispatchCSMCombine(EcsImpl::EntityManager& manager, GameContext& context);

		AssetHandle<ComputeShader> m_Combine = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "CSMCombine");
		AssetHandle<Texture2D> m_ShadowCombined;
		AssetHandle<Texture2D> m_ShadowCombined2;
		glm::uvec2 m_Res{ 0u };
		f32 m_DepthRejectBlur = 0.2f;
	};
}
