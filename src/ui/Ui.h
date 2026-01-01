#pragma once

#include "../core/Global.h"
#include "../ecs/GameContext.h"


#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "../../ecs/systems/Systems.h"


namespace EnGl
{
	class Ui
	{
	public:
		
		struct State
		{
			
		};

		State m_State;
		ImGuiIO* m_io;

		Ui(GLFWwindow* window);
		~Ui();

		void Render(GameContext& context, EcsImpl::EntityManager& manager, EcsImpl::SystemRegistry<GameContext>& systemRegistry);
		void Present();
	private:
		GLFWwindow* m_Window;

		void Frame(GameContext& context, EcsImpl::EntityManager& manager);

		void CameraView(GameContext::CameraInfo& cameraInfo, EcsImpl::EntityManager& manager);
		void DebugView(GameContext::DebugInfo &debug, EcsImpl::EntityManager& manager);
	};
}