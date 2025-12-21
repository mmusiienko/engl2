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

		struct ViewProjectionMatrix
		{
			ViewProjectionMatrix() { UpdateProjection(); }

			glm::mat4 CachedView;
			glm::mat4 CachedProjection;

			void UpdateProjection(
				f32 Aspect = 16.0f / 9.0f,
				f32 FovDegree = 45.0f,
				f32 NearPlane = 0.01f,
				f32 FarPlane = 10000.0f
			)
			{
				CachedProjection = glm::perspective(glm::radians(FovDegree), Aspect, NearPlane, FarPlane);
			}
		};

		struct Movement
		{
			f32 Velocity;
			glm::vec3 NormalizedDirection;

			void SetDirection(const glm::vec3& direction)
			{
				auto length = glm::length(direction);
				NormalizedDirection = (length < Constants::TOLERANCE) ? direction : direction / length;
			}
		};

		enum RenderLayer
		{
			NORMAL,
			SCREEN_QUAD,
			SCREEN_SPACE,

			Count
		};

		constexpr u32 RenderLayerNumber = RenderLayer::Count;

		struct RenderedModel
		{
			AssetHandle<Model> Model;
			u32 MeshIdx = 0;
			RenderLayer Layer = RenderLayer::NORMAL;
		};
	}
	
	using EcsImpl = Ecs
	<
		Component::Transform,
		Component::ModelMatrix,
		Component::ViewProjectionMatrix,
		Component::Movement,
		Component::RenderedModel
	>;
}