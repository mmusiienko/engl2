#pragma once

#include "../../core/Constants.h"
#include "../../resources/importers/AssetManager.h"
#include "../../renderer/base/Model.h"

#include "../entity.h"


namespace EnGl
{
	namespace Component
	{
		struct Transform
		{
			glm::vec3 Position = glm::vec3(0.0f);
			glm::quat Rotation { 1.0f, 0.0f, 0.0f, 0.0f};
			glm::vec3 Scale = glm::vec3(1.0f);

			bool Dirty = true;
			bool OnlyPosDirty = true;
		};

		struct ModelMatrix
		{
			glm::mat4 CachedModel;
			bool Dirty = true;
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
			glm::mat4 CachedView;
		};

		struct ProjectionMatrix
		{
			glm::mat4 CachedProjection;
		};

		struct Velocity
		{
			f32 Speed;
			glm::vec3 NormalizedDirection;

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

		constexpr u32 RenderLayerNumber = RenderLayer::Count;

		struct RenderedModel
		{
			AssetHandle<Model> Model{};
			i32 MeshIdx = -1;
			RenderLayer Layer = RenderLayer::OQ;
		};

		struct PointLight
		{
			glm::vec3 Color;
			float Intensity;
		};

		struct DirectionalLight
		{
			glm::vec3 Color;
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
		Component::DirectionalLight
	>;
}