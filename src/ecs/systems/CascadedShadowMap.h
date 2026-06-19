#pragma once
#include "ecs/systems/Systems.h"


namespace EnGl::System
{
	class CascadedShadowMap : public SystemImpl
	{
		void Run(EcsImpl::EntityManager& manager, GameContext& context) override;
		void Editor(EcsImpl::EntityManager& manager, GameContext& context) override;
	};
}
