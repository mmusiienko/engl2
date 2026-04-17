#pragma once

#include "Core.h"

#include "renderer/base/Framebuffer.h"

#include "ecs/entity.h"
#include "ecs/components/Components.h"


namespace EnGl 
{
	struct InputInfo
	{
		glm::vec2 MouseDelta;
		glm::vec3 MovementDirection;
	};

	class Application
	{
	public:
		Application();
		~Application();
		static void framebuffer_size_callback(GLFWwindow* window, int width, int height);

		EcsImpl& World()
		{
			return m_Ecs;
		}

		InputInfo& GetInputInfo()
		{
			return info;
		}

		void Run();
	private:
		f64 m_PauseTime = 0.0f;
		GLFWwindow* m_Window = nullptr;
		scope<Framebuffer> m_Framebuffer = nullptr;

		Entity m_Camera = 0;
		EcsImpl m_Ecs;
		InputInfo info;
		void CreateFramebuffer(u32 w, u32 h);
		void ResizeFramebuffer(u32 w, u32 h);
		void InitGl();
		void InitWindow();
	};
}
