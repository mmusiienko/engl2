#pragma once

#include "../ecs/GameContext.h"

#include "EcsUi.h"


namespace EnGl
{
	class Ui
	{
	public:
		ImGuiIO* m_io;

		Ui(GLFWwindow* window);
		~Ui();

		void Render(GameContext& context, EcsImpl::EntityManager& manager, EcsImpl::SystemRegistry<GameContext>& systemRegistry);
		void Present();
	private:
		GLFWwindow* m_Window;

		struct State
		{
			enum PanelState { NONE, ECS, Count } CurrentPanelState = NONE;
		};

		State m_State;

		EcsUi m_EcsUi{};

		void Frame(GameContext& context, EcsImpl::EntityManager& manager, EcsImpl::SystemRegistry<GameContext>& registry);

		void DebugView(GameContext::DebugInfo &debug, EcsImpl::EntityManager& manager);
	};
}