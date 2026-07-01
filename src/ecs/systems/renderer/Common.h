#pragma once
#include "ecs/systems/Systems.h"


namespace EnGl::System
{
	namespace Renderer::Common
	{
		void RenderLayer(EcsImpl::EntityManager& manager, GameContext& context, u32 layer, RenderpassType type = RenderpassType::NORMAL);
	};
}
