#pragma once
#include "ecs/systems/Systems.h"

namespace EnGl::System
{
	class PlayAnimation : public SystemImpl
	{
		void Run(EcsImpl::EntityManager& manager, GameContext& context) override;
	};

	class CollectAnimationTransforms : public SystemImpl
	{
		void Run(EcsImpl::EntityManager& manager, GameContext& context) override;
	};
}

namespace EnGl::Component
{
	void AddAnimationData(EcsImpl::EntityManager& manager, Entity e, AssetHandle<Animation> anim, AssetHandle<Skeleton> skel);
}