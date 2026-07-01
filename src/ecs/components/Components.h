#pragma once

#include "core/Constants.h"
#include "resources/importers/AssetManager.h"
#include "renderer/base/Model.h"
#include "renderer/base/Animation.h"
#include "renderer/base/Skeleton.h"

#include "ecs/entity.h"

namespace EnGl
{
	namespace Component
	{
		struct Transform
		{
			glm::vec3 Position = glm::vec3{ 0.0f };
			glm::quat Rotation{ 1.0f, 0.0f, 0.0f, 0.0f };
			glm::vec3 Scale = glm::vec3{ 1.0f };

			bool Dirty = true;
			bool OnlyPosDirty = true;
		};

		struct ModelMatrix
		{
			glm::mat4 CachedModel{};
		};

		struct OrthogonalProjection
		{
			f32 NearPlane = 0.1f;
			f32 FarPlane = 100000.0f;
			f32 Right = 20.0f;
			f32 Top = 20.0f;
			f32 Left = -20.0f;
			f32 Bottom = -20.0f;
			bool Dirty = true;
		};

		struct PerspectiveProjection
		{
			f32 Aspect = 16.0f / 9.0f;
			f32 FovDegree = 45.0f;
			f32 NearPlane = 0.1f;
			f32 FarPlane = 100000.0f;
			bool Dirty = true;
		};

		struct ViewMatrix
		{
			glm::mat4 CachedView{};
		};

		struct ProjectionMatrix
		{
			glm::mat4 CachedProjection{};
		};

		struct Velocity
		{
			f32 Speed = 0.0f;
			glm::vec3 NormalizedDirection{};

			void SetDirection(const glm::vec3& direction)
			{
				auto length = glm::length(direction);
				NormalizedDirection = (length < Constants::TOLERANCE) ? direction : direction / length;
			}
		};

		enum RenderLayer
		{
			OQ,
			TT,
			OL,
			CUBEMAP,
			SCREEN_QUAD,
			SCREEN_SPACE,
			Count
		};

		constexpr inline u32 RenderLayerNumber = RenderLayer::Count;

		struct RenderedModel
		{
			AssetHandle<Model> Model{};
			i32 MeshIdx = -1;
			RenderLayer Layer = RenderLayer::OQ;
		};

		struct PointLight
		{
			glm::vec3 Color{ 1.0f };
			float Intensity = 100.0f;
		};

		struct DirectionalLight
		{
			glm::vec3 Color{ 1.0f };
		};

		struct SphereCollider
		{
			glm::vec3 Offset{};
			f32 Radius = 1.0f;
		};

		struct PhysicalMomentum
		{
			glm::vec3 Velocity{};
			f32 InverseMass = 1.0f;
			f32 Restitution = 1.0f;
		};

		struct LengthConstraint
		{
			Entity E1 = 0;
			Entity E2 = 0;
			f32 Length = 1.0f;
		};

		struct MaxLengthConstraint
		{
			Entity E1 = 0;
			Entity E2 = 0;
			f32 MaxLength = 1.0f;
		};

		struct FollowSnap
		{
			f32 Snap = 1.0f;
			Entity Follow = 0u;
			glm::vec3 PosOffset{};
			glm::bvec3 PosUnlock{ true };
		};

		struct ConstantRotation
		{
			glm::vec3 Velocity{};
		};

		struct Children
		{
			std::vector<Entity> Children;
		};

		struct Parent
		{
			Entity Parent;
		};

		struct LocalModelMatrix
		{
			glm::mat4 CachedModel{};
			bool Dirty = false;
		};

		

		struct AnimationData
		{
			enum class AnimationBehavior : u32
			{
				LOOP,
				CLAMP,
				COUNT
			};

			AnimationBehavior Behavior = AnimationBehavior::CLAMP;

			f32 Elapsed = 0.0f;
			f32 Speed = 1.0f;
			AssetHandle<AnimationInstance> Animation;
			AssetHandle<SkeletonInstance> Skeleton;
		};
	}
	
	using EcsImpl = Ecs
	<
		Component::Transform,
		Component::ModelMatrix,
		Component::PerspectiveProjection,
		Component::OrthogonalProjection,
		Component::ViewMatrix,
		Component::ProjectionMatrix,
		Component::Velocity,
		Component::RenderedModel,
		Component::PointLight,
		Component::DirectionalLight,
		Component::SphereCollider,
		Component::PhysicalMomentum,
		Component::LengthConstraint,
		Component::MaxLengthConstraint,
		Component::FollowSnap,
		Component::ConstantRotation,
		Component::Parent,
		Component::Children,
		Component::LocalModelMatrix,
		Component::AnimationData
	>;
}