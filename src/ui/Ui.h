#pragma once

#include "../core/Global.h"
#include "../ecs/GameContext.h"

#include "ImGuiEntry.h"
#include "../../ecs/systems/Systems.h"


namespace EnGl
{
	class Ui
	{
	public:
		
		struct State
		{
			enum PanelState {NONE, ENTITY, SYSTEM, Count} CurrentPanelState = NONE;
			
			struct EntityPanelInfo
			{
				EcsImpl::CSignature FilterSignature;
				EcsImpl::Entity SelectedEntity;
				bool IsEntitySelected = false;
			} EntityPanel {};

			struct SystemPanelInfo
			{
				size_t SelectedSystem;
				bool IsSystemSelected = false;
			} SystemPanel{};
		};

		State m_State;
		ImGuiIO* m_io;

		Ui(GLFWwindow* window);
		~Ui();

		void Render(GameContext& context, EcsImpl::EntityManager& manager, EcsImpl::SystemRegistry<GameContext>& systemRegistry);
		void Present();
	private:
		GLFWwindow* m_Window;

		void Frame(GameContext& context, EcsImpl::EntityManager& manager, EcsImpl::SystemRegistry<GameContext>& registry);

		void EntityPanel(GameContext& context, EcsImpl::EntityManager& manager);
		void SystemPanel(GameContext& context, EcsImpl::EntityManager& manager, EcsImpl::SystemRegistry<GameContext>& registry);

		void EntityListView(GameContext& context, EcsImpl::EntityManager& manager);
		void EntityView(EcsImpl::Entity entity, GameContext& context, EcsImpl::EntityManager& manager);

		void SystemListView(GameContext& context, EcsImpl::EntityManager& manager, EcsImpl::SystemRegistry<GameContext>& registry);
		void SystemView(size_t system, GameContext& context, EcsImpl::EntityManager& manager, EcsImpl::SystemRegistry<GameContext>& registry);

		void TransformView(Component::Transform& transform);
		void VelocityView(Component::Velocity& velocity);
		void PerspectiveProjectionView(Component::PerspectiveProjection& viewProj, GameContext::CameraInfo& cameraInfo);
		void OrthogonalProjectionView(Component::OrthogonalProjection& viewProj, GameContext::CameraInfo& cameraInfo);
		void PointLightView(Component::PointLight& point);
		void DirectionalLightView(Component::DirectionalLight& dir);
		void ModelView(Component::RenderedModel& model);

		void DebugView(GameContext::DebugInfo &debug, EcsImpl::EntityManager& manager);
	};
}