#pragma once

#include "../core/Global.h"
#include "../ecs/GameContext.h"


#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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

		void Render(GameContext& context);
		void Present();
	private:
		GLFWwindow* m_Window;

		void Frame(GameContext& context);

		void CameraView(GameContext::CameraInfo& cameraInfo);
	};
}