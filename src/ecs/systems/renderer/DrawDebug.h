#pragma once
#include "ecs/systems/Systems.h"


namespace EnGl::System
{
	class DrawDebug : public SystemImpl
	{
		void Run(EcsImpl::EntityManager& manager, GameContext& context) override;
		AssetHandle<Shader> m_DebugShader = AssetManager::Load<Shader>(AssetManager::GRAPHICS_SHADER_DIR / "DebugLines");
	};
}
