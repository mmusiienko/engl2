#include "Ui.h"
#include "spdlog/spdlog.h"
#include <ranges>


namespace EnGl
{
	Ui::Ui(GLFWwindow* window) : m_Window(window)
	{
		spdlog::info("Initializing imgui");

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		m_io = &ImGui::GetIO();
		m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		m_io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

		ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
		ImGui_ImplOpenGL3_Init();
	}

	Ui::~Ui()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void Ui::Render(GameContext& context)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		Frame(context);

		ImGui::Render();
	}

	void Ui::Present()
	{
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void Ui::Frame(GameContext& context)
	{
		ImGui::Begin("Configuration");

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_io->Framerate, m_io->Framerate);

		ImGui::Separator();

		CameraView(context.Camera);

		ImGui::End();
	}

	void Ui::CameraView(GameContext::CameraInfo& cameraInfo)
	{
		ImGui::Text("Selected Camera Id: %d", cameraInfo.Get());
		for (const auto [idx, camId] : std::views::enumerate(cameraInfo.Cameras))
		{
			auto label = "Camera Id: " + std::to_string(camId) + "##SelectCamera" + std::to_string(idx);
			if (ImGui::Button(label.c_str()))
			{
				cameraInfo.CameraIdx = idx;
			}
		}
	}
}