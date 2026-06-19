#pragma once

#include "ui/ImGuiEntry.h"

#include "ecs/systems/Systems.h"
#include "resources/StaticModel.h"
#include "ui/Components.h"


namespace EnGl
{
	class EcsUi
	{
		struct State
		{
			struct EntityPanelInfo
			{
				EcsImpl::CSignature FilterSignature{ true };
				Entity SelectedEntity;
				bool IsEntitySelected = false;
				u32 Page = 0;
				u32 EntitiesPerPage = 10;
			} EntityPanel{};

			struct SystemPanelInfo
			{
				size_t SelectedSystem;
				bool IsSystemSelected = false;
			} SystemPanel{};

			enum PanelState { NONE, ENTITY, SYSTEM, Count } CurrentPanelState = NONE;
		} m_State;

	public:
		void Frame(GameContext& context, EcsImpl::EntityManager& manager, EcsImpl::SystemRegistry<GameContext>& registry)
		{
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

	private:
		template<typename AssetT>
		static void ChangeAsset(AssetHandle<AssetT>& assetHandle)
		{
			if (ImGui::Button("Change asset"))
			{
				ImGui::OpenPopup("change_asset");
			}

			if (ImGui::BeginPopup("change_asset"))
			{
				for (auto res : AssetManager::ListAssets<AssetT>())
				{
					auto label = std::format("{}, {}", AssetManager::GetName(res), res.Id);
					if (ImGui::Selectable(label.c_str()))
					{
						assetHandle = res;
					}
				}
				ImGui::EndPopup();
			}
		}

		void EntityPanel(GameContext& context, EcsImpl::EntityManager& manager)
		{
			EntityListView(context, manager);

			if (m_State.EntityPanel.IsEntitySelected)
			{
				EntityView(m_State.EntityPanel.SelectedEntity, context, manager);
			}
		}

		void SystemPanel(GameContext& context, EcsImpl::EntityManager& manager, EcsImpl::SystemRegistry<GameContext>& registry)
		{
			SystemListView(context, manager, registry);

			if (m_State.SystemPanel.IsSystemSelected)
			{
				SystemView(m_State.SystemPanel.SelectedSystem, context, manager, registry);
			}
		}

		void EntityListView(GameContext& context, EcsImpl::EntityManager& manager)
		{
			ImGui::Separator();

			ImGui::Text("Entities");

			auto it = manager.Query(m_State.EntityPanel.FilterSignature);
			auto end = it.end();
			u32 i = 0;

			while (it != end && i < m_State.EntityPanel.Page * m_State.EntityPanel.EntitiesPerPage)
			{
				++it;
				++i;
			}

			i = 0;
			while (it != end && i < m_State.EntityPanel.EntitiesPerPage)
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

			AddEntityBtn(manager);

			PageSelect(manager);

			ImGui::Separator();
		}

		void PageSelect(EcsImpl::EntityManager& manager)
		{
			u32 total = static_cast<u32>(glm::ceil(static_cast<f32>(manager.Count()) / m_State.EntityPanel.EntitiesPerPage));
			for (u32 i = 0; i < total; i++)
			{
				ImGui::PushID(i);

				ImGui::SameLine();
				auto str = std::to_string(i);
				if (ImGui::Button(str.c_str()))
				{
					m_State.EntityPanel.Page = i;
				}

				ImGui::PopID();
			}
		}

		void EntityView(Entity entity, GameContext& context, EcsImpl::EntityManager& manager)
		{
			ImGui::Text("Selected Entity: %s, id: %d", manager.GetName(entity).c_str(), entity);

			u32 i = 0;
			std::apply([&]<typename... Cs>(Cs...)
			{
				((manager.Has<Cs>(entity)
					? ComponentView<Cs>(entity, context, manager, i)
					: void()), ...);
			}, UI_SUPPORTED);

			AddComponentBtn(entity, manager);
			RemoveEntityBtn(entity, manager);
		}

		template<ECSComponent C>
		void ComponentView(Entity entity, GameContext& context, EcsImpl::EntityManager& manager, u32& i)
		{
			ImGui::PushID(i);
			ImGui::Separator();
			ImGui::Text("%s Component", Name<C>().data());
			View<C>(manager.Get<C>(entity), context, manager, entity);
			RemoveComponentBtn<C>(entity, manager);
			ImGui::PopID();
			i++;
		}

		template<ECSComponent C>
		void RemoveComponentBtn(Entity entity, EcsImpl::EntityManager& manager)
		{
			if (ImGui::Button("- Remove Component"))
			{
				manager.Remove<C>(entity);
			}
		}

		void AddComponentBtn(Entity entity, EcsImpl::EntityManager& manager)
		{
			static const std::string_view popupId = "add_component_btn_popup";

			if (ImGui::Button("+ Add Component"))
			{
				ImGui::OpenPopup(popupId.data());
			}

			if (ImGui::BeginPopup(popupId.data()))
			{
				u32 i = 0;
				std::apply([&]<typename... Cs>(Cs...)
				{
					((!manager.Has<Cs>(entity)
						? AddComponentOption<Cs>(entity, manager, i)
						: void()), ...);
				}, UI_SUPPORTED);

				ImGui::EndPopup();
			}
		}

		template<ECSComponent C>
		void AddComponentOption(Entity entity, EcsImpl::EntityManager& manager, u32& i)
		{
			std::string label = std::format("{} component", Name<C>().data());
			if (ImGui::Selectable(label.c_str()))
			{
				manager.Add<C>(entity);
			}
		}

		void RemoveEntityBtn(Entity entity, EcsImpl::EntityManager& manager)
		{
			if (ImGui::Button("--- Remove Entity"))
			{
				manager.Remove(entity);
			}
		}

		void AddEntityBtn(EcsImpl::EntityManager& manager)
		{
			if (ImGui::Button("+ Add entity"))
			{
				manager.Create<Component::Transform>();
			}
		}

		void SystemListView(GameContext& context, EcsImpl::EntityManager& manager, EcsImpl::SystemRegistry<GameContext>& registry)
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

		void SystemView(size_t system, GameContext& context, EcsImpl::EntityManager& manager, EcsImpl::SystemRegistry<GameContext>& registry)
		{
			ImGui::Text("Selected System: %s", registry.GetName(system).c_str());
			ImGui::Spacing();
			registry.Get(system)->Editor(manager, context);
		}

		std::tuple<
			Component::Transform,
			Component::ModelMatrix,
			Component::PerspectiveProjection,
			Component::OrthogonalProjection,
			Component::ViewMatrix,
			Component::ProjectionMatrix,
			Component::Velocity,
			Component::RenderedModel,
			Component::PointLight,
			Component::DirectionalLight,
			Component::SphereCollider,
			Component::PhysicalMomentum,
			Component::LengthConstraint,
			Component::MaxLengthConstraint,
			Component::ConstantRotation,
			Component::AnimationData,
			Component::Children
		> UI_SUPPORTED;

		template<ECSComponent C> static const std::string_view Name() { return "Component"; }
		template<> static const std::string_view Name<Component::Transform>() { return "Transform"; }
		template<> static const std::string_view Name<Component::ModelMatrix>() { return "Model Matrix"; }
		template<> static const std::string_view Name<Component::PerspectiveProjection>() { return "Perspective Projection"; }
		template<> static const std::string_view Name<Component::OrthogonalProjection>() { return "Orthogonal Projection"; }
		template<> static const std::string_view Name<Component::ViewMatrix>() { return "View Matrix"; }
		template<> static const std::string_view Name<Component::ProjectionMatrix>() { return "Projection Matrix"; }
		template<> static const std::string_view Name<Component::Velocity>() { return "Velocity"; }
		template<> static const std::string_view Name<Component::RenderedModel>() { return "Rendered Model"; }
		template<> static const std::string_view Name<Component::PointLight>() { return "Point Light"; }
		template<> static const std::string_view Name<Component::DirectionalLight>() { return "Directional Light"; }
		template<> static const std::string_view Name<Component::SphereCollider>() { return "Sphere Collider"; }
		template<> static const std::string_view Name<Component::PhysicalMomentum>() { return "Physical Momentum"; }
		template<> static const std::string_view Name<Component::LengthConstraint>() { return "Length Constraint"; }
		template<> static const std::string_view Name<Component::MaxLengthConstraint>() { return "Max Length Constraint"; }
		template<> static const std::string_view Name<Component::ConstantRotation>() { return "Constant Rotation"; }
		template<> static const std::string_view Name<Component::AnimationData>() { return "Animation Data"; }
		template<> static const std::string_view Name<Component::Children>() { return "Children"; }

		template<ECSComponent C> void View(C& component, GameContext& context, EcsImpl::EntityManager& manager, Entity entity) {}

		//void EditTransform(float* cameraView, float* cameraProjection, float* matrix)
		//{
		//	static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::ROTATE);
		//	static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::WORLD);
		//	if (ImGui::IsKeyPressed(ImGuiKey_T))
		//		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		//	if (ImGui::IsKeyPressed(ImGuiKey_E))
		//		mCurrentGizmoOperation = ImGuizmo::ROTATE;
		//	if (ImGui::IsKeyPressed(ImGuiKey_R))
		//		mCurrentGizmoOperation = ImGuizmo::SCALE;
		//	if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
		//		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		//	ImGui::SameLine();
		//	if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
		//		mCurrentGizmoOperation = ImGuizmo::ROTATE;
		//	ImGui::SameLine();
		//	if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
		//		mCurrentGizmoOperation = ImGuizmo::SCALE;
		//	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
		//	ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
		//	ImGui::InputFloat3("Tr", matrixTranslation);
		//	ImGui::InputFloat3("Rt", matrixRotation);
		//	ImGui::InputFloat3("Sc", matrixScale);
		//	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);

		//	if (mCurrentGizmoOperation != ImGuizmo::SCALE)
		//	{
		//		if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
		//			mCurrentGizmoMode = ImGuizmo::LOCAL;
		//		ImGui::SameLine();
		//		if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
		//			mCurrentGizmoMode = ImGuizmo::WORLD;
		//	}
		//	static bool useSnap(false);
		//	if (ImGui::IsKeyPressed(ImGuiKey_S))
		//		useSnap = !useSnap;
		//	ImGui::Checkbox("##useSnap", &useSnap);
		//	ImGui::SameLine();
		//	glm::vec3 snap;

		//	static const glm::vec3 SNAP{};
		//	switch (mCurrentGizmoOperation)
		//	{
		//	case ImGuizmo::TRANSLATE:
		//		snap.x = SNAP.x;
		//		ImGui::InputFloat3("Snap", &snap.x);
		//		break;
		//	case ImGuizmo::ROTATE:
		//		snap.y = SNAP.y;
		//		ImGui::InputFloat("Angle Snap", &snap.y);
		//		break;
		//	case ImGuizmo::SCALE:
		//		snap.z = SNAP.z;
		//		ImGui::InputFloat("Scale Snap", &snap.z);
		//		break;
		//	default:
		//		break;
		//	}
		//	ImGuiIO& io = ImGui::GetIO();
		//	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
		//	ImGuizmo::Manipulate(cameraView, cameraProjection, mCurrentGizmoOperation,
		//		mCurrentGizmoMode, matrix, NULL, useSnap ? &snap.x : NULL);
		//}

		template<>
		void View<Component::Transform>(Component::Transform& transform, GameContext& context, EcsImpl::EntityManager& manager, Entity entity)
		{
			const f32 DIST = 10.0f;
			auto [camTransform, model] = manager.Get<Component::Transform, Component::ModelMatrix>(context.Camera.GetEntity());

			if (ImGui::Button("Jump to"))
			{
				if (!manager.Has<Component::LocalModelMatrix>(entity))
				{
					camTransform.Position = transform.Position - DIST * context.Camera.Get().Forward;
				}
				else if (manager.Has<Component::ModelMatrix>(entity))
				{
					camTransform.Position = glm::vec3{ manager.Get<Component::ModelMatrix>(entity).CachedModel[3] } - DIST * context.Camera.Get().Forward;
				}
			}

			transform.Dirty |= ImGui::InputFloat3("Position", glm::value_ptr(transform.Position));
			transform.Dirty |= ImGui::InputFloat4("RotationQuat", glm::value_ptr(transform.Rotation));

			glm::vec3 euler = glm::eulerAngles(transform.Rotation);

			euler = glm::degrees(euler);
			euler = glm::mod(euler + 180.0f, 360.0f) - 180.0f;

			bool changed = ImGui::DragFloat3("RotationEulerAngles", &euler.x, 0.1f);
			transform.Dirty |= changed;

			if (changed)
			{
				glm::vec3 rad = glm::radians(euler);
				transform.Rotation = glm::quat(rad);
			}

			transform.Dirty |= ImGui::InputFloat3("Scale", glm::value_ptr(transform.Scale));

			//EditTransform(
			//	glm::value_ptr(context.Camera.Get().View),
			//	glm::value_ptr(context.Camera.Get().Projection),
			//	glm::value_ptr(model.CachedModel)
			//);

			//glm::vec3 euler = glm::degrees(glm::eulerAngles(transform.Rotation));
			//ImGuizmo::DecomposeMatrixToComponents(
			//	glm::value_ptr(model.CachedModel),
			//	glm::value_ptr(transform.Position),
			//	glm::value_ptr(euler),
			//	glm::value_ptr(transform.Scale)
			//);
			//transform.Rotation = glm::quat(glm::radians(euler));
		}

		template<>
		void View<Component::Velocity>(Component::Velocity& velocity, GameContext& context, EcsImpl::EntityManager& manager, Entity entity)
		{
			ImGui::InputFloat("Speed", &velocity.Speed);
		}

		void SelectCameraBtn(Entity entity, GameContext::CameraInfo& cameraInfo)
		{
			auto label = std::format("Select camera##SelectCamera");
			if (ImGui::Button(label.c_str()))
			{
				cameraInfo.SetCamera(entity);
			}
		}

		template<>
		void View<Component::PerspectiveProjection>(Component::PerspectiveProjection& persp, GameContext& context, EcsImpl::EntityManager& manager, Entity entity)
		{
			SelectCameraBtn(m_State.EntityPanel.SelectedEntity, context.Camera);

			persp.Dirty |= ImGui::InputFloat("Aspect", &persp.Aspect);
			persp.Dirty |= ImGui::InputFloat("Fov", &persp.FovDegree);
			persp.Dirty |= ImGui::InputFloat("Near", &persp.NearPlane);
			persp.Dirty |= ImGui::InputFloat("Far", &persp.FarPlane);
		}

		template<>
		void View<Component::OrthogonalProjection>(Component::OrthogonalProjection& ortho, GameContext& context, EcsImpl::EntityManager& manager, Entity entity)
		{
			SelectCameraBtn(m_State.EntityPanel.SelectedEntity, context.Camera);

			ortho.Dirty |= ImGui::InputFloat("Right", &ortho.Right);
			ortho.Dirty |= ImGui::InputFloat("Top", &ortho.Top);
			ortho.Dirty |= ImGui::InputFloat("Left", &ortho.Left);
			ortho.Dirty |= ImGui::InputFloat("Bottom", &ortho.Bottom);
			ortho.Dirty |= ImGui::InputFloat("Near", &ortho.NearPlane);
			ortho.Dirty |= ImGui::InputFloat("Far", &ortho.FarPlane);
		}

		template<>
		void View<Component::PointLight>(Component::PointLight& point, GameContext& context, EcsImpl::EntityManager& manager, Entity entity)
		{
			ImGui::ColorEdit3("Color", glm::value_ptr(point.Color));
			ImGui::InputFloat("Intensity", &point.Intensity);
		}

		template<>
		void View<Component::DirectionalLight>(Component::DirectionalLight& dir, GameContext& context, EcsImpl::EntityManager& manager, Entity entity)
		{
			ImGui::ColorEdit3("Color", glm::value_ptr(dir.Color));
		}

		static void MeshView(AssetHandle<Mesh>& meshH)
		{
			auto mesh = AssetManager::GetAsset(meshH).Asset;

			if (!mesh)
			{
				ImGui::Text("Mesh asset not loaded");
			}
			ImGui::Text("Mesh");
			ChangeAsset<Mesh>(meshH);

			if (ImGui::Button("Select static mesh"))
			{
				ImGui::OpenPopup("change_mesh");
			}

			if (ImGui::BeginPopup("change_mesh"))
			{
				if (ImGui::Selectable("Cube"))
					meshH = StaticMesh::Cube();
				if (ImGui::Selectable("Sphere"))
					meshH = StaticMesh::Sphere();
				if (ImGui::Selectable("Quad"))
					meshH = StaticMesh::Quad();

				ImGui::EndPopup();
			}
		}

		static void MaterialView(AssetHandle<Material::Base>& materialH)
		{
			auto material = AssetManager::GetAsset(materialH).Asset;

			if (!material)
			{
				ImGui::Text("Material asset not loaded");
				return;
			}
			ImGui::Text("Material");

			material->Editor();

			ChangeAsset<Material::Base>(materialH);

			if (ImGui::Button("+ New material"))
			{
				ImGui::OpenPopup("new_material");
			}

			if (ImGui::BeginPopup("new_material"))
			{
				if (ImGui::Selectable("PBR"))
					materialH = AssetManager::PutScope<Material::Base>(make_scope<Material::PBR>());
				if (ImGui::Selectable("Lit"))
					materialH = AssetManager::PutScope<Material::Base>(make_scope<Material::Lit>());
				if (ImGui::Selectable("Unlit"))
					materialH = AssetManager::PutScope<Material::Base>(make_scope<Material::Unlit>());
				if (ImGui::Selectable("ScreenSpace"))
					materialH = AssetManager::PutScope<Material::Base>(make_scope<Material::ScreenSpaceTextured>());

				ImGui::EndPopup();
			}
		}

		static void SubmeshView(Model::Submesh& submesh)
		{
			ImGui::PushID(0);
			MeshView(submesh.Mesh);
			ImGui::PopID();

			ImGui::PushID(1);
			MaterialView(submesh.Material);
			ImGui::PopID();
		}

		template<>
		void View<Component::RenderedModel>(Component::RenderedModel& modelC, GameContext& context, EcsImpl::EntityManager& manager, Entity entity)
		{
			ImGui::Text("Layer %d", modelC.Layer);
			ImGui::Text("MeshIdx %d", modelC.MeshIdx);
			auto model = AssetManager::GetAsset(modelC.Model).Asset;

			if (!model)
			{
				ImGui::Text("Model asset not loaded");
				ChangeAsset<Model>(modelC.Model);
				return;
			}

			u32 idx = modelC.MeshIdx == -1 ? 0 : static_cast<u32>(modelC.MeshIdx);
			u32 lim = modelC.MeshIdx == -1 ? model->TotalMeshes() : idx + 1;
			for (; idx < lim; idx++)
			{
				ImGui::PushID(idx);
				auto& submesh = model->GetSubmesh(idx);
				SubmeshView(submesh);
				ImGui::PopID();
			}
		}

		template<>
		void View<Component::SphereCollider>(Component::SphereCollider& collider, GameContext& context, EcsImpl::EntityManager& manager, Entity entity)
		{
			ImGui::InputFloat3("Offset", glm::value_ptr(collider.Offset));
			ImGui::InputFloat("Radius", &collider.Radius);
		}

		template<>
		void View<Component::PhysicalMomentum>(Component::PhysicalMomentum& momentum, GameContext& context, EcsImpl::EntityManager& manager, Entity entity)
		{
			ImGui::InputFloat3("Velocity", glm::value_ptr(momentum.Velocity));
			ImGui::InputFloat("Inverse mass", &momentum.InverseMass);
			ImGui::InputFloat("Restitution", &momentum.Restitution);
		}

		template<>
		void View<Component::LengthConstraint>(Component::LengthConstraint& constraint, GameContext& context, EcsImpl::EntityManager& manager, Entity entity)
		{
			ImGui::InputFloat("Length", &constraint.Length);
			UiComponents::InputUInt("E1", &constraint.E1);
			UiComponents::InputUInt("E2", &constraint.E2);
		}

		template<>
		void View<Component::MaxLengthConstraint>(Component::MaxLengthConstraint& constraint, GameContext& context, EcsImpl::EntityManager& manager, Entity entity)
		{
			ImGui::InputFloat("Max Length", &constraint.MaxLength);
			UiComponents::InputUInt("E1", &constraint.E1);
			UiComponents::InputUInt("E2", &constraint.E2);
		}

		template<>
		void View<Component::ConstantRotation>(Component::ConstantRotation& rot, GameContext& context, EcsImpl::EntityManager& manager, Entity entity)
		{
			ImGui::InputFloat3("Velocity", glm::value_ptr(rot.Velocity));
		}

		template<>
		void View<Component::AnimationData>(Component::AnimationData& data, GameContext& context, EcsImpl::EntityManager& manager, Entity entity)
		{
			ImGui::InputFloat("Elapsed", &data.Elapsed);
			ImGui::SliderFloat("Speed", &data.Speed, 0.0f, 10.0f);

			if (ImGui::Button("Loop"))
			{
				data.Behavior = Component::AnimationData::AnimationBehavior::LOOP;
			}
			if (ImGui::Button("Clamp"))
			{
				data.Behavior = Component::AnimationData::AnimationBehavior::CLAMP;
			}
		}

		static void RecurseChildren(EcsImpl::EntityManager& manager, Entity entity)
		{
			ImGui::PushID(entity);

			if (manager.Has<Component::Children>(entity))
			{
				bool open = ImGui::TreeNode(manager.GetName(entity).c_str());
				if (open)
				{
					for (auto& child : manager.Get<Component::Children>(entity).Children)
						RecurseChildren(manager, child);
					ImGui::TreePop();
				}
			}
			else
			{
				ImGui::TreeNodeEx(manager.GetName(entity).c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
			}

			ImGui::PopID();
		}

		template<>
		void View<Component::Children>(Component::Children& data, GameContext& context, EcsImpl::EntityManager& manager, Entity entity)
		{
			ImGui::Separator();
			ImGui::Text("Hierarchy");
			RecurseChildren(manager, entity);
		}
	};
}