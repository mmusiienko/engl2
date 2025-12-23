#pragma once

#include "../entity.h"
#include "../components/components.h"
#include "../../core/Global.h"
#include "../GameContext.h"
#include <ranges>


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

		class CameraIntersectWorld : public SystemImpl
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

		class Input : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto [transform, movement] = manager.Get<Component::Transform, Component::Movement>(context.Camera.Get().Entity);
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
				if (InputHandler::State.KeysHeld[GLFW_KEY_R])
					AssetManager::ReloadAll<Shader>();

				if (InputHandler::State.KeysPressed[GLFW_KEY_F1])
					context.Debug.DrawMode = GL_FILL;

				if (InputHandler::State.KeysPressed[GLFW_KEY_F2])
					context.Debug.DrawMode = GL_LINE;

				if (context.Camera.Get().CanRotate)
				{
					if (glm::length(InputHandler::State.MouseDelta) > Constants::TOLERANCE)
					{
						static f32 sens = 0.002f;
						float yawAngle = -InputHandler::State.MouseDelta.x * sens;
						float pitchAngle = -InputHandler::State.MouseDelta.y * sens;

						glm::quat yawQuat = glm::angleAxis(yawAngle, glm::vec3(0, 1, 0));
						glm::quat pitchQuat = glm::angleAxis(pitchAngle, transform.Rotation * Constants::RIGHT);

						transform.Rotation = glm::normalize(yawQuat * pitchQuat * transform.Rotation);
					}

					direction = transform.Rotation * direction;
				}


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

				for (auto& camera : context.Camera.Cameras)
				{
					auto [viewProj, transform] = manager.Get<Component::ViewProjectionMatrix, Component::Transform>(camera.Entity);

					camera.Position = &transform.Position;
					camera.Forward = transform.Rotation * Constants::FORWARD;
					camera.View = &viewProj.CachedView;
					camera.InverseView = glm::inverse(viewProj.CachedView);
					camera.Projection = &viewProj.CachedProjection;
					camera.InverseProjection = glm::inverse(viewProj.CachedProjection);
					camera.ViewProjection = viewProj.CachedProjection * viewProj.CachedView;
					camera.InverseViewProjection = glm::inverse(camera.ViewProjection);
				}
			}
		};

		class PrepareDebugDraw : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context)
			{
				if (context.Debug.Draw.Enabled && context.Debug.Draw.Camera)
				{
					static const glm::vec4 tln{ -1.0f, 1.0f, -1.0f, 1.0f };
					static const glm::vec4 bln{ -1.0f, -1.0f, -1.0f, 1.0f };
					static const glm::vec4 trn{ 1.0f, 1.0f, -1.0f, 1.0f };
					static const glm::vec4 brn{ 1.0f, -1.0f, -1.0f, 1.0f };
					static f32 farDistDebug = 50.0f;
					static f32 forwardDistDebug = farDistDebug * 2.0f;

					for (const auto& [idx, camera] : std::views::enumerate(context.Camera.Cameras))
					{
						if (camera.Entity == context.Camera.Get().Entity) continue;

						context.Debug.DebugMeshes.Line(*camera.Position, *camera.Position + forwardDistDebug * camera.Forward, DebugMesh::DEFAULT_COLOR, 0.1f);

						auto nearPlaneTl = camera.InverseViewProjection * tln;
						auto nearPlaneBl = camera.InverseViewProjection * bln;
						auto nearPlaneTr = camera.InverseViewProjection * trn;
						auto nearPlaneBr = camera.InverseViewProjection * brn;

						auto tl = glm::vec3{ nearPlaneTl / nearPlaneTl.w };
						auto bl = glm::vec3{ nearPlaneBl / nearPlaneBl.w };
						auto tr = glm::vec3{ nearPlaneTr / nearPlaneTr.w };
						auto br = glm::vec3{ nearPlaneBr / nearPlaneBr.w };

						auto ftl = tl + camera.Forward * farDistDebug;
						auto fbl = bl + camera.Forward * farDistDebug;
						auto ftr = tr + camera.Forward * farDistDebug;
						auto fbr = br + camera.Forward * farDistDebug;

						glm::vec3 center =
							(tl + bl + tr + br) * 0.25f;

						glm::vec3 fcenter =
							center + camera.Forward * farDistDebug;

						f32 scale = 10.0f;

						tl = center + glm::normalize(tl - center) * scale;
						tr = center + glm::normalize(tr - center) * scale;
						bl = center + glm::normalize(bl - center) * scale;
						br = center + glm::normalize(br - center) * scale;

						f32 fscale = 25.0f;

						ftl = fcenter + glm::normalize(ftl - fcenter) * fscale;
						ftr = fcenter + glm::normalize(ftr - fcenter) * fscale;
						fbl = fcenter + glm::normalize(fbl - fcenter) * fscale;
						fbr = fcenter + glm::normalize(fbr - fcenter) * fscale;

						context.Debug.DebugMeshes.FrameQuad(fbl, ftl, ftr, fbr, DebugMesh::DEFAULT_COLOR, 0.1f);
						context.Debug.DebugMeshes.Line(*camera.Position, ftl, DebugMesh::DEFAULT_COLOR, 0.1f);
						context.Debug.DebugMeshes.Line(*camera.Position, ftr, DebugMesh::DEFAULT_COLOR, 0.1f);
						context.Debug.DebugMeshes.Line(*camera.Position, fbl, DebugMesh::DEFAULT_COLOR, 0.1f);
						context.Debug.DebugMeshes.Line(*camera.Position, fbr, DebugMesh::DEFAULT_COLOR, 0.1f);

					}
				}
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

					if (context.Debug.Draw.Enabled && context.Debug.Draw.AABB)
					{
						auto aabb = mesh->GetAABB();
						auto mn = glm::vec3{ transform.Model * glm::vec4{ aabb.Min, 1.0f } };
						auto mx = glm::vec3{ transform.Model * glm::vec4{ aabb.Max, 1.0f } };
						context.Debug.DebugMeshes.FrameCube(mn, mx);
					}
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

				if (context.Debug.Draw.Enabled && context.Debug.Draw.AABB)
				{
					auto aabb = mesh->GetAABB();
					for (const auto& transform : value.Transforms)
					{
						auto mn = glm::vec3{ transform * glm::vec4{ aabb.Min, 1.0f } };
						auto mx = glm::vec3{ transform * glm::vec4{ aabb.Max, 1.0f } };
						context.Debug.DebugMeshes.FrameCube(mn, mx);
					}
				}

				material->SetUniforms();
				mesh->DrawInstanced();
			}
		}

		class RenderToFramebuffer : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context)
			{
				glPolygonMode(GL_FRONT_AND_BACK, context.Debug.DrawMode);

				glDisable(GL_BLEND);
				glDepthMask(GL_TRUE);

				RenderLayer(context, Component::RenderLayer::OQ);

				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glDepthMask(GL_FALSE);

				RenderLayer(context, Component::RenderLayer::TT);

				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

				RenderLayer(context, Component::RenderLayer::OL);

				glPolygonMode(GL_FRONT_AND_BACK, context.Debug.DrawMode);
				glDisable(GL_BLEND);
				glDepthMask(GL_TRUE);

				if (context.Debug.Draw.Enabled)
				{
					auto [shader, g] = AssetManager::GetAsset(m_DebugShader);
					if (shader)
					{
						shader->Use();
						shader->SetUniform("uViewProjection", context.Camera.Get().ViewProjection);
						context.Debug.DebugMeshes.Draw();
					}
				}
			}

			AssetHandle<Shader> m_DebugShader = AssetManager::Load<Shader>(AssetManager::GRAPHICS_SHADER_DIR / "DebugLines");
		};

		class RenderToDefaultFramebuffer : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context)
			{
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

				RenderLayer(context, Component::RenderLayer::SCREEN_QUAD);

				glPolygonMode(GL_FRONT_AND_BACK, context.Debug.DrawMode);

				for (u32 i = Component::RenderLayer::SCREEN_QUAD + 1; i < Component::RenderLayer::Count; i++)
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
				GL_CHECK( glClearColor(0.0f, 0.0f, 0.0f, 1.0f) );
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

				context.Debug.DebugMeshes.Tick();
			}	
		};
	}
}
