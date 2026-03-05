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

	void Ui::Render(GameContext& context, EcsImpl::EntityManager& manager, EcsImpl::SystemRegistry<GameContext>& registry)
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

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

		if (ImGui::Button("Entity Panel"))
			m_State.CurrentPanelState = State::PanelState::ENTITY;

		if (ImGui::Button("System Panel"))
			m_State.CurrentPanelState = State::PanelState::SYSTEM;

		switch (m_State.CurrentPanelState)
		{
		case State::PanelState::ENTITY:
			EntityPanel(context, manager);
			break;
		case State::PanelState::SYSTEM:
			SystemPanel(context, manager, registry);
			break;
		case State::PanelState::NONE:
			break;
		default:
			break;
		}
	}

	void Ui::EntityPanel(GameContext& context, EcsImpl::EntityManager& manager)
	{
		EntityListView(context, manager);

		if (m_State.EntityPanel.IsEntitySelected)
		{
			EntityView(m_State.EntityPanel.SelectedEntity, context, manager);
		}
	}

	void Ui::SystemPanel(GameContext& context, EcsImpl::EntityManager& manager, EcsImpl::SystemRegistry<GameContext>& registry)
	{
		SystemListView(context, manager, registry);

		if (m_State.SystemPanel.IsSystemSelected)
		{
			SystemView(m_State.SystemPanel.SelectedSystem, context, manager, registry);
		}
	}

	void Ui::EntityListView(GameContext& context, EcsImpl::EntityManager& manager)
	{
		ImGui::Separator();

		ImGui::Text("Entities");

		auto it = manager.Query(m_State.EntityPanel.FilterSignature);
		auto end = it.end();
		u32 i = 0;
		while (it != end && i < 10)
		{
			auto label = std::format("Name: {}, Id: {}##SelectEntity{}", manager.GetName(*it), *it, *it);
			if (ImGui::Button(label.c_str()))
			{
				m_State.EntityPanel.SelectedEntity = *it;
				m_State.EntityPanel.IsEntitySelected = true;
			}
			++it;
			++i;
		}

		ImGui::Separator();
	}

	void Ui::EntityView(EcsImpl::Entity entity, GameContext& context, EcsImpl::EntityManager& manager)
	{
		ImGui::Text("Selected Entity: %s, id: %d", manager.GetName(entity).c_str(), entity);

		if (manager.Has<Component::Transform>(entity))
			TransformView(manager.Get<Component::Transform>(entity));

		if (manager.Has<Component::Velocity>(entity))
			VelocityView(manager.Get<Component::Velocity>(entity));

		if (manager.Has<Component::PerspectiveProjection>(entity))
			PerspectiveProjectionView(manager.Get<Component::PerspectiveProjection>(entity), context.Camera);

		if (manager.Has<Component::OrthogonalProjection>(entity))
			OrthogonalProjectionView(manager.Get<Component::OrthogonalProjection>(entity), context.Camera);

		if (manager.Has<Component::PointLight>(entity))
			PointLightView(manager.Get<Component::PointLight>(entity));

		if (manager.Has<Component::DirectionalLight>(entity))
			DirectionalLightView(manager.Get<Component::DirectionalLight>(entity));

		if (manager.Has<Component::RenderedModel>(entity))
			ModelView(manager.Get<Component::RenderedModel>(entity));
	}

	void Ui::SystemListView(GameContext& context, EcsImpl::EntityManager& manager, EcsImpl::SystemRegistry<GameContext>& registry)
	{
		for (const auto& [id, _] : std::views::enumerate(registry.Systems()))
		{
			auto label = std::format("Name: {}##SelectSystem{}", registry.GetName(id), id);
			if (ImGui::Button(label.c_str()))
			{
				m_State.SystemPanel.SelectedSystem = id;
				m_State.SystemPanel.IsSystemSelected = true;
			}
		}
	}

	void Ui::SystemView(size_t system, GameContext& context, EcsImpl::EntityManager& manager, EcsImpl::SystemRegistry<GameContext>& registry)
	{
		ImGui::Text("Selected System: %s", registry.GetName(system).c_str());
		ImGui::Spacing();
		registry.Get(system)->Editor(manager, context);
	}

	void Ui::DebugView(GameContext::DebugInfo& debug, EcsImpl::EntityManager& manager)
	{
		ImGui::Separator();
		ImGui::Text("Debug flags");
		ImGui::Checkbox("Enable", &debug.Draw.Enabled);
		ImGui::Checkbox("Draw Cameras", &debug.Draw.Camera);
		ImGui::Checkbox("Draw Bounding Boxes", &debug.Draw.AABB);
	}

	void Ui::TransformView(Component::Transform& transform)
	{
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Text("Transform component");
		ImGui::Spacing();

		transform.Dirty |= ImGui::InputFloat3("Position", glm::value_ptr(transform.Position));
		transform.Dirty |= ImGui::InputFloat4("Rotation", glm::value_ptr(transform.Rotation));
		transform.Dirty |= ImGui::InputFloat3("Scale", glm::value_ptr(transform.Scale));

		ImGui::Separator();
	}

	void Ui::VelocityView(Component::Velocity& velocity)
	{
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Text("Velocity component");
		ImGui::Spacing();

		ImGui::InputFloat("Speed", &velocity.Speed);

		ImGui::Separator();
	}

	void Ui::PerspectiveProjectionView(Component::PerspectiveProjection& persp, GameContext::CameraInfo& cameraInfo)
	{
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Text("Perspective Projection component");
		ImGui::Spacing();

		auto label = std::format("Select camera##SelectCamera");
		if (ImGui::Button(label.c_str()))
		{
			auto it = std::find_if(
				cameraInfo.Cameras.begin(), cameraInfo.Cameras.end(),
				[=](auto& c) -> bool {return m_State.EntityPanel.SelectedEntity == c.Entity; }
			);
			cameraInfo.CameraIdx = it - cameraInfo.Cameras.begin();
		}

		persp.Dirty |= ImGui::InputFloat("Aspect", &persp.Aspect);
		persp.Dirty |= ImGui::InputFloat("Fov", &persp.FovDegree);
		persp.Dirty |= ImGui::InputFloat("Near", &persp.NearPlane);
		persp.Dirty |= ImGui::InputFloat("Far", &persp.FarPlane);
	}

	void Ui::OrthogonalProjectionView(Component::OrthogonalProjection& ortho, GameContext::CameraInfo& cameraInfo)
	{
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Text("Orthogonal Projection component");
		ImGui::Spacing();

		auto label = std::format("Select camera##SelectCamera");
		if (ImGui::Button(label.c_str()))
		{
			auto it = std::find_if(
				cameraInfo.Cameras.begin(), cameraInfo.Cameras.end(),
				[=](auto& c) -> bool {return m_State.EntityPanel.SelectedEntity == c.Entity; }
			);
			cameraInfo.CameraIdx = it - cameraInfo.Cameras.begin();
		}

		ortho.Dirty |= ImGui::InputFloat("Right", &ortho.Right);
		ortho.Dirty |= ImGui::InputFloat("Top", &ortho.Top);
		ortho.Dirty |= ImGui::InputFloat("Left", &ortho.Left);
		ortho.Dirty |= ImGui::InputFloat("Bottom", &ortho.Bottom);
		ortho.Dirty |= ImGui::InputFloat("Near", &ortho.NearPlane);
		ortho.Dirty |= ImGui::InputFloat("Far", &ortho.FarPlane);
	}

	void Ui::PointLightView(Component::PointLight& point)
	{
		ImGui::Separator();
		ImGui::Text("Point light component");
		ImGui::ColorEdit3("Color", glm::value_ptr(point.Color));
		ImGui::InputFloat("Intensity", &point.Intensity);
	}

	void Ui::DirectionalLightView(Component::DirectionalLight& dir)
	{
		ImGui::Separator();
		ImGui::Text("Directional light component");
		ImGui::ColorEdit3("Color", glm::value_ptr(dir.Color));
	}

	static void MeshView(AssetHandle<Mesh> meshA)
	{
		auto mesh = AssetManager::GetAsset(meshA).Asset;

		if (!mesh)
		{
			ImGui::Text("Mesh asset not loaded");
			return;
		}

		ImGui::Text("Mesh id: %d", mesh->Id());
	}

	static void MaterialView(AssetHandle<scope<Material::Base>> materialA)
	{
		auto material = AssetManager::GetAsset(materialA).Asset;

		if (!material)
		{
			ImGui::Text("Material asset not loaded");
			return;
		}

		material->Editor();
	}

	static void SubmeshView(Model::Submesh& submesh)
	{
		MeshView(submesh.Mesh);
		MaterialView(submesh.Material);
	}

	void Ui::ModelView(Component::RenderedModel& modelC)
	{
		ImGui::Separator();
		ImGui::Text("Model component");
		ImGui::Text("Layer %d", modelC.Layer);
		ImGui::Text("MeshIdx %d", modelC.MeshIdx);
		auto model = AssetManager::GetAsset(modelC.Model).Asset;

		if (!model)
		{
			ImGui::Text("Model asset not loaded");
			return;
		}

		size_t idx = modelC.MeshIdx == -1 ? 0 : modelC.MeshIdx;
		size_t lim = modelC.MeshIdx == -1 ? model->TotalMeshes() : idx + 1;
		for (; idx < lim; idx++)
		{
			ImGui::PushID(idx);
			auto& submesh = model->GetSubmesh(idx);
			SubmeshView(submesh);
			ImGui::PopID();
		}
	}
}