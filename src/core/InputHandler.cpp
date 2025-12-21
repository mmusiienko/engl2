#include "InputHandler.h"


namespace EnGl
{
	void InputHandler::CursorCallback(GLFWwindow* window, f64 xpos, f64 ypos)
	{
		int mode = glfwGetInputMode(window, GLFW_CURSOR);

		if (mode != GLFW_CURSOR_DISABLED)
			return;

		auto pos = glm::vec2{ static_cast<f32>(xpos), static_cast<f32>(ypos) };
		if (State.FirstMouse)
		{
			State.LastMousePosition = pos;
			State.FirstMouse = false;
		}

		State.MouseDelta = pos - State.LastMousePosition;
		State.LastMousePosition = pos;
	}

	void InputHandler::MouseCallback(GLFWwindow* window, int button, int action, int mods)
	{
		if (ImGui::GetIO().WantCaptureMouse)
			return;

		if (action == GLFW_PRESS)
			State.MouseBPressed[button] = true;
		else if (action == GLFW_RELEASE)
			State.MouseBReleased[button] = true;

		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			f64 xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			State.LastMousePosition = { static_cast<f32>(xpos), static_cast<f32>(ypos) };
			State.FirstMouse = true;
		}
	}

	void InputHandler::KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (ImGui::GetIO().WantCaptureKeyboard)
			return;

		if (action == GLFW_PRESS)
			State.KeysPressed[key] = true;
		else if (action == GLFW_RELEASE)
			State.KeysReleased[key] = true;

		if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

		if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			f64 xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			State.LastMousePosition = { static_cast<f32>(xpos), static_cast<f32>(ypos) };
			State.FirstMouse = true;
		}
	}

	void InputHandler::KeyboardEvent(GLFWwindow* window)
	{

		for (size_t i = 0; i < MAX_KEYS; i++)
		{
			if (glfwGetKey(window, i) == GLFW_PRESS)
				State.KeysHeld[i] = true;
		}
	}

	void InputHandler::MouseEvent(GLFWwindow* window)
	{
		for (size_t i = 0; i < MAX_MOUSE_BUTTONS; i++)
		{
			if (glfwGetMouseButton(window, i) == GLFW_PRESS)
				State.MouseBHeld[i] = true;
		}
	}

	void InputHandler::ProcessInputEvents(GLFWwindow* window)
	{
		if (!ImGui::GetIO().WantCaptureMouse)
			MouseEvent(window);
		if (!ImGui::GetIO().WantCaptureKeyboard)
			KeyboardEvent(window);
	}

	void InputHandler::ResetState()
	{
		for (size_t i = 0; i < MAX_KEYS; i++)
		{
			State.KeysHeld[i] = false;
			State.KeysPressed[i] = false;
			State.KeysReleased[i] = false;
		}

		for (size_t i = 0; i < MAX_MOUSE_BUTTONS; i++)
		{
			State.MouseBHeld[i] = false;
			State.MouseBReleased[i] = false;
			State.MouseBPressed[i] = false;
		}
	}
}