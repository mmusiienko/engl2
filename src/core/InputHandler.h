#pragma once
#include "Core.h"
#include "../math/Math.h"
#include "imgui_impl_glfw.h"


namespace EnGl
{
	struct InputHandler
	{
		static constexpr inline u32 MAX_KEYS = GLFW_KEY_LAST + 1;
		static constexpr inline u32 MAX_MOUSE_BUTTONS = GLFW_MOUSE_BUTTON_LAST + 1;

		struct State
		{
			bool FirstMouse = true;
			glm::vec2 LastMousePosition{};
			glm::vec2 MouseDelta{};
			f32 ScrollDelta = 0.0f;

			bool KeysHeld[MAX_KEYS]{};
			bool KeysPressed[MAX_KEYS]{};
			bool KeysReleased[MAX_KEYS]{};

			bool MouseBHeld[MAX_MOUSE_BUTTONS]{};
			bool MouseBReleased[MAX_MOUSE_BUTTONS]{};
			bool MouseBPressed[MAX_MOUSE_BUTTONS]{};
		};

		inline static State State;
		static void CursorCallback(GLFWwindow* window, f64 xpos, f64 ypos);
		static void MouseCallback(GLFWwindow* window, int button, int action, int mods);
		static void ScrollCallback(GLFWwindow* window, f64 xoffset, f64 yoffset);

		static void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void KeyboardEvent(GLFWwindow* window);
		static void MouseEvent(GLFWwindow* window);
		static void ProcessInputEvents(GLFWwindow* window);
		static void ResetState();
	};
}
