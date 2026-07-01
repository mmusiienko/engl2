#include "ui/Ui.h"

#include "spdlog/spdlog.h"


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

	void Ui::Render(GameContext& context, EcsImpl::EntityManager& manager, EcsImpl::SystemRegistry<GameContext>& registry)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		//ImGuizmo::BeginFrame();

		Frame(context, manager, registry);

		ImGui::End();
		ImGui::Render();
	}

	void Ui::Present()
	{
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void Ui::Frame(GameContext& context, EcsImpl::EntityManager& manager, EcsImpl::SystemRegistry<GameContext>& registry)
	{
		ImGui::Begin("Configuration");

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / m_io->Framerate, m_io->Framerate);

		ImGui::Separator();
		DebugView(context.Debug, manager);

		m_EcsUi.Frame(context, manager, registry);
	}

	void Ui::DebugView(GameContext::DebugInfo& debug, EcsImpl::EntityManager& manager)
	{
		ImGui::Separator();
		ImGui::Text("Debug flags");
		ImGui::Checkbox("Enable", &debug.Draw.Enabled);
		ImGui::Checkbox("Draw Cameras", &debug.Draw.Camera);
		ImGui::Checkbox("Draw Bounding Boxes", &debug.Draw.AABB);
	}
}