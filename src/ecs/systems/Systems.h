#pragma once

#include "../entity.h"
#include "../components/components.h"
#include "../../core/Global.h"
#include "../GameContext.h"


namespace EnGl
{
	

	using SystemImpl = EcsImpl::System<GameContext>;
	using SystemRegistryImpl = EcsImpl::SystemRegistry<GameContext>;

	namespace System
	{
		class ModelMatrix : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto query = manager.Query<Component::Transform, Component::ModelMatrix>();
				u32 i = 0;

				for (auto [e, transform, model] : query)
				{
					if (transform.Dirty)
					{
						UpdateModelMatrix(model.CachedModel, transform);

						model.Dirty = true;
					} else if (transform.OnlyPosDirty)
					{
						UpdateModelOnlyPos(model.CachedModel, transform.Position);

						model.Dirty = true;
					}

					i++;
				}
			}

			static void UpdateModelMatrix(glm::mat4& original, const Component::Transform& transform)
			{
				glm::mat4 model = glm::mat4(1.0f);
				model = glm::scale(model, transform.Scale);
				model *= glm::mat4_cast(transform.Rotation);
				model = glm::translate(model, transform.Position);

				original = std::move(model);
			}

			static void UpdateModelOnlyPos(glm::mat4& original, const glm::vec3& pos)
			{
				original[3][0] = pos.x;
				original[3][1] = pos.y;
				original[3][2] = pos.z;
			}
		};

		class Movement : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto query = manager.Query<Component::Transform, Component::Movement>();

				for (auto [e, transform, movement] : query)
				{
					transform.Position += context.DeltaTime * movement.Velocity * movement.NormalizedDirection;
					transform.OnlyPosDirty = true;
				}
			}
		};

		class CameraInput : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto& movement = manager.Get<Component::Movement>(context.Camera.Get());
				glm::vec3 direction{};
				if (InputHandler::State.KeysHeld[GLFW_KEY_W])
					direction.z -= 1;
				if (InputHandler::State.KeysHeld[GLFW_KEY_S])
					direction.z += 1;
				if (InputHandler::State.KeysHeld[GLFW_KEY_A])
					direction.x -= 1;
				if (InputHandler::State.KeysHeld[GLFW_KEY_D])
					direction.x += 1;
				if (InputHandler::State.KeysHeld[GLFW_KEY_SPACE])
					direction.y += 1;
				if (InputHandler::State.KeysHeld[GLFW_KEY_LEFT_CONTROL])
					direction.y -= 1;

				movement.SetDirection(direction);
			}
		};

		class ViewProjectionMatrix : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto query = manager.Query<Component::Transform, Component::ViewProjectionMatrix>();

				for (auto [e, transform, viewProjection] : query)
				{
					viewProjection.CachedView = glm::mat4_cast(glm::conjugate(transform.Rotation)) *
						glm::translate(glm::mat4(1.0f), -transform.Position);
				}

				auto& viewProj = manager.Get<Component::ViewProjectionMatrix>(context.Camera.Get());
				
				context.Camera.View = &viewProj.CachedView;
				context.Camera.Projection = &viewProj.CachedProjection;
			}
		};

		class BundleDirtyMaterials : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context)
			{
				auto query = manager.Query<Component::RenderedModel, Component::ModelMatrix>();
				
				for (auto [e, modelComp, transform] : query)
				{
					auto [model, gen] = AssetManager::GetAsset(modelComp.Model);
					
					if (modelComp.MeshIdx == model->TotalMeshes())
					{
						for (size_t i = 0; i < modelComp.MeshIdx; i++)
						{
							AddToMaterialMap(context, model, i, transform, modelComp.Layer);
						}
					}
					else
					{
						AddToMaterialMap(context, model, modelComp.MeshIdx, transform, modelComp.Layer);
					}
					
				}
			}

			static void AddToMaterialMap(
				GameContext& context,
				Model* model,
				size_t idx,
				Component::ModelMatrix& modelMatrix,
				Component::RenderLayer layer
			)
			{
				if (model->IsInstanced())
				{
					auto& map = context.Renderer.PerInstancedMaterial.at(layer);
					auto submesh = model->GetSubmesh(idx);
					auto& bucket = map[
						GameContext::InstancedMaterialMapKey{
							.MeshHandle = submesh.Mesh,
							.MaterialHandle = submesh.Material
						}
					];
					bucket.Transforms.push_back(modelMatrix.CachedModel);
				}
				else
				{
					auto& map = context.Renderer.PerMaterial.at(layer);

					auto submesh = model->GetSubmesh(idx);
					auto& bucket = map[submesh.Material];
					bucket.Transforms.push_back({.Mesh = submesh.Mesh, .Model = modelMatrix.CachedModel});
				}
			}
		};

		static void RenderLayer(GameContext& context, u32 layer)
		{
			for (auto& [key, value] : context.Renderer.PerMaterial[layer])
			{
				auto material = AssetManager::GetAssetNoCheck(key);

				bool ok = material->SetCommonUniforms(context);
				if (!ok)
				{
					spdlog::error("Error setting material.");
				}

				for (auto& transform : value.Transforms)
				{
					auto mesh = AssetManager::GetAssetNoCheck(transform.Mesh);

					material->SetModel(transform.Model);
					material->SetUniforms();
					mesh->Draw();
				}
			}

			for (auto& [key, value] : context.Renderer.PerInstancedMaterial[layer])
			{
				auto mesh = AssetManager::GetAssetNoCheck(key.MeshHandle);
				auto material = AssetManager::GetAssetNoCheck(key.MaterialHandle);

				bool ok = material->SetCommonUniforms(context);
				if (!ok)
				{
					spdlog::error("Error setting material.");
				}
				
				mesh->UpdateInstanceBuffer(value.Transforms);

				material->SetUniforms();
				mesh->DrawInstanced();
			}
		}

		class RenderToFramebuffer : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context)
			{
				for (u32 i = 0; i < Component::RenderLayer::NORMAL + 1; i++)
				{
					RenderLayer(context, i);
				}
			}
		};

		class RenderToDefaultFramebuffer : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context)
			{
				for (u32 i = Component::RenderLayer::NORMAL + 1; i < Component::RenderLayer::Count; i++)
				{
					RenderLayer(context, i);
				}
			}
		};


		class BindDefaultFramebuffer : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context)
			{
				GL_CHECK( glBindFramebuffer(GL_FRAMEBUFFER, 0) );
				GL_CHECK( glClearColor(0.0f, 0.0f, 0.0f, 0.0f) );
				GL_CHECK( glClear(GL_COLOR_BUFFER_BIT) );
				GL_CHECK( glDisable(GL_DEPTH_TEST) );
			}
		};

		class Cleanup : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context)
			{
				auto queryTransform = manager.Query<Component::Transform>();
				auto queryModelMatrix = manager.Query<Component::ModelMatrix>();
				for (auto [e, transform] : queryTransform)
				{
					transform.Dirty = false;
					transform.OnlyPosDirty = false;
				}

				for (auto [e, model] : queryModelMatrix)
				{
					model.Dirty = false;
				}

				for (u32 layer = 0; layer < Component::RenderLayer::Count; layer++)
				{
					context.Renderer.PerInstancedMaterial[layer].clear();
					context.Renderer.PerMaterial[layer].clear();
				}
			}	
		};
	}
}
