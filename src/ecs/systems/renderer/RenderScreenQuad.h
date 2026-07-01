#pragma once
#include "ecs/systems/Systems.h"


namespace EnGl::System
{
	class RenderScreenQuad : public SystemImpl
	{
		void Run(EcsImpl::EntityManager& manager, GameContext& context) override;
	};
}
