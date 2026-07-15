#pragma once
#include "ecs/systems/Systems.h"


namespace EnGl::System
{
	class Prepass : public SystemImpl
	{
		void Init(EcsImpl::EntityManager& manager) override;
		void Run(EcsImpl::EntityManager& manager, GameContext& context) override;
		void Editor(EcsImpl::EntityManager& manager, GameContext& context) override;

		AssetHandle<Texture2D> m_DepthWithoutTransparents{};
		glm::uvec2 m_Res{ 1u };
	};
}
