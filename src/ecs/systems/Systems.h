#pragma once

#include "../entity.h"
#include "../components/components.h"
#include "../GameContext.h"
#include <ranges>
#include <utility>
#include "../../core/Constants.h"
#include "../../core/Core.h"
#include "../../core/InputHandler.h"
#include "../../renderer/base/Model.h"
#include "../../renderer/base/Mesh.h"
#include "../../renderer/base/Material.h"
#include "../../renderer/base/Shader.h"
#include "../../resources/DebugMesh.h"
#include "../../resources/importers/AssetHandle.h"
#include "../../resources/importers/AssetManager.h"


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
				model = glm::translate(model, transform.Position);
				model *= glm::mat4_cast(transform.Rotation);
				model = glm::scale(model, transform.Scale);

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
				auto query = manager.Query<Component::Transform, Component::Velocity>();

				for (auto [e, transform, velocity] : query)
				{
					transform.Position += context.DeltaTime * velocity.Speed * velocity.NormalizedDirection;
					transform.OnlyPosDirty = true;
				}
			}
		};

		class CameraIntersectWorld : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto query = manager.Query<Component::Transform, Component::Velocity>();

				for (auto [e, transform, velocity] : query)
				{
					transform.Position += context.DeltaTime * velocity.Speed * velocity.NormalizedDirection;
					transform.OnlyPosDirty = true;
				}
			}
		};

		class Input : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto [transform, movement] = manager.Get<Component::Transform, Component::Velocity>(context.Camera.Get().Entity);
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
				if (InputHandler::State.KeysHeld[GLFW_KEY_K])
					AssetManager::ReloadAll<ComputeShader>();

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
						transform.Dirty = true;
					}
				}

				movement.SetDirection(transform.Rotation * direction);
			}
		};

		class ViewMatrix : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto query = manager.Query<Component::Transform, Component::ViewMatrix>();

				for (auto [e, transform, view] : query)
				{
					view.CachedView = glm::mat4_cast(glm::conjugate(transform.Rotation)) *
						glm::translate(glm::mat4(1.0f), -transform.Position);
				}
			}
		};

		class ProjectionMatrix : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto queryPersp = manager.Query<Component::ProjectionMatrix, Component::PerspectiveProjection>();
				auto queryOrtho = manager.Query<Component::ProjectionMatrix, Component::OrthogonalProjection>();

				for (auto [e, proj, persp] : queryPersp)
				{
					if (persp.Dirty)
					{
						proj.CachedProjection = glm::perspective(
							glm::radians(persp.FovDegree),
							persp.Aspect,
							persp.NearPlane,
							persp.FarPlane
						);
						persp.Dirty = false;
					}
				}

				for (auto [e, proj, ortho] : queryOrtho)
				{
					if (ortho.Dirty)
					{
						proj.CachedProjection = glm::ortho(
							ortho.Left,
							ortho.Right,
							ortho.Bottom,
							ortho.Top,
							ortho.NearPlane,
							ortho.FarPlane
						);
						ortho.Dirty = false;
					}
				}
			}
		};

		class UpdateCameras : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				for (auto& camera : context.Camera.Cameras)
				{
					auto [view, proj, transform] = manager.Get<
						Component::ViewMatrix,
						Component::ProjectionMatrix,
						Component::Transform
					>(camera.Entity);

					camera.Position = &transform.Position;
					camera.Forward = transform.Rotation * Constants::FORWARD;
					camera.View = &view.CachedView;
					camera.InverseView = glm::inverse(view.CachedView);
					camera.Projection = &proj.CachedProjection;
					camera.InverseProjection = glm::inverse(proj.CachedProjection);
					camera.ViewProjection = proj.CachedProjection * view.CachedView;
					camera.InverseViewProjection = glm::inverse(camera.ViewProjection);

					if (manager.Has<Component::PerspectiveProjection>(camera.Entity))
					{
						auto persp = manager.Get<Component::PerspectiveProjection>(camera.Entity);
						camera.Near = persp.NearPlane;
						camera.Far = persp.FarPlane;
					}
					else if (manager.Has<Component::OrthogonalProjection>(camera.Entity))
					{
						auto persp = manager.Get<Component::OrthogonalProjection>(camera.Entity);
						camera.Near = persp.NearPlane;
						camera.Far = persp.FarPlane;
					}
				}
			}
		};

		class CollectLights : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto queryD = manager.Query<Component::Transform, Component::DirectionalLight>();
				auto queryP = manager.Query<Component::Transform, Component::PointLight>();

				for (auto [e, t, d] : queryD)
				{
					auto dir = -glm::normalize(t.Rotation * Constants::FORWARD);
					context.DirLight = { .Direction = dir, .Color = d.Color };
					auto dirCamera = context.Camera.GetDirShadowCamera().Entity;
					auto& dirCamTransform = manager.Get<Component::Transform>(dirCamera);

					dirCamTransform.Rotation = t.Rotation;
					dirCamTransform.Dirty = true;

					auto camera = context.Camera.Get().Entity;
					if (camera != dirCamera)
					{
						const auto& camTransform = manager.Get<Component::Transform>(camera);

						dirCamTransform.Position = camTransform.Position + dir * m_DirLightCameraDist;

						if (context.Debug.Draw.Enabled)
						{
							static constexpr auto WIDTH = glm::vec3{ 5.0f };
							context.Debug.DebugMeshes.Line(dirCamTransform.Position, dirCamTransform.Position + dir * m_DirLightCameraDist);
						}
					}
					
					break;
				}

				size_t i = 0;
				for (auto [e, t, p] : queryP)
				{
					context.PointLights[i++] = { .Position = t.Position, .Color = p.Color, .Intensity = p.Intensity };
					if (context.Debug.Draw.Enabled)
					{
						static constexpr auto WIDTH = glm::vec3{ 5.0f };
						context.Debug.DebugMeshes.FrameCube(t.Position - WIDTH, t.Position + WIDTH);
					}

					if (i >= Shader::MAX_LIGHTS) break;
				}
			}

			f32 m_DirLightCameraDist = 200.0f;
		};

		class PrepareDebugDraw : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
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
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto query = manager.Query<Component::RenderedModel, Component::ModelMatrix>();
				for (auto [e, modelComp, transform] : query)
				{
					auto [model, gen] = AssetManager::GetAsset(modelComp.Model);

					if (!model)
					{
						spdlog::error("Model to be rendered does not exist.");
						continue;
					}

					if (modelComp.MeshIdx == -1)
					{
						for (u32 i = 0; i < model->TotalMeshes(); i++)
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
					bucket.Data.push_back({
						.Model = modelMatrix.CachedModel,
						.Normal = glm::transpose(glm::inverse(modelMatrix.CachedModel))
						});
				}
				else
				{
					auto& map = context.Renderer.PerMaterial.at(layer);

					auto submesh = model->GetSubmesh(idx);
					auto material = AssetManager::GetAsset(submesh.Material).Asset;

					if (!material)
					{
						spdlog::error("Material is not loaded.");
						return;
					}

					auto& bucket = map[material->Name()];
					bucket.InstanceDatas.push_back(
						{
							.Mesh = submesh.Mesh,
							.Material = submesh.Material,
							.Data =
							{
								.Model = modelMatrix.CachedModel,
								.Normal = glm::transpose(glm::inverse(modelMatrix.CachedModel))
							}
						}
					);
				}
			}
		};

		static void RenderLayer(GameContext& context, u32 layer)
		{
			auto materialOverride = AssetManager::GetAsset(context.Renderer.MaterialOverride).Asset;
			bool isMaterialOverridden = (materialOverride != nullptr);

			for (auto& [key, value] : context.Renderer.PerMaterial[layer])
			{
				auto material = (isMaterialOverridden) ? materialOverride : AssetManager::GetAssetNoCheck(value.InstanceDatas[0].Material);

				bool ok = material->SetCommonUniforms(context);
				if (!ok)
				{
					spdlog::error("Error setting material.");
				}

				for (auto& data : value.InstanceDatas)
				{
					auto mesh = AssetManager::GetAssetNoCheck(data.Mesh);

					material->SetModel(data.Data.Model, data.Data.Normal);
					material->SetUniforms();
					mesh->Draw();

					if (context.Debug.Draw.Enabled && context.Debug.Draw.AABB)
					{
						auto aabb = mesh->GetAABB();
						auto mn = glm::vec3{ data.Data.Model * glm::vec4{ aabb.Min, 1.0f } };
						auto mx = glm::vec3{ data.Data.Model * glm::vec4{ aabb.Max, 1.0f } };
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

				mesh->UpdateInstanceBuffer(value.Data);

				if (context.Debug.Draw.Enabled && context.Debug.Draw.AABB)
				{
					auto aabb = mesh->GetAABB();
					for (const auto& [model, _] : value.Data)
					{
						auto mn = glm::vec3{ model * glm::vec4{ aabb.Min, 1.0f } };
						auto mx = glm::vec3{ model * glm::vec4{ aabb.Max, 1.0f } };
						context.Debug.DebugMeshes.FrameCube(mn, mx);
					}
				}

				material->SetUniforms();
				mesh->DrawInstanced();
			}
		}

		class RenderToFramebuffer : public SystemImpl
		{
			void Init(EcsImpl::EntityManager& manager)
			{
				auto unlit = make_scope<Material::Unlit>();
				unlit->Color = {};
				m_Unlit = AssetManager::Put<scope<Material::Base>>(
					std::move(unlit)
				);
			}

			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				glPolygonMode(GL_FRONT_AND_BACK, context.Debug.DrawMode);
				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);

				context.Framebuffer.DirShadowFramebuffer->Bind();
				glClear(GL_DEPTH_BUFFER_BIT);
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
				context.Renderer.MaterialOverride = m_Unlit;
				std::swap(context.Camera.CameraIdx, context.Camera.DirShadowCameraIdx);
				RenderLayer(context, Component::RenderLayer::OQ);
				std::swap(context.Camera.CameraIdx, context.Camera.DirShadowCameraIdx);
				context.Renderer.MaterialOverride = {};

				context.Framebuffer.MainFramebuffer->Bind();
				glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				RenderLayer(context, Component::RenderLayer::OQ);

				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_LEQUAL);
				glDepthMask(GL_FALSE);
				glCullFace(GL_FRONT);

				RenderLayer(context, Component::RenderLayer::CUBEMAP);
				glDepthMask(GL_TRUE);
				glCullFace(GL_BACK);

				//copy depth
				auto depth = AssetManager::GetAsset(context.Framebuffer.MainFramebuffer->Depth()).Asset;
				auto depthOpaque = AssetManager::GetAsset(context.Framebuffer.DepthTextureOpaque).Asset;

				if (!depth)
					spdlog::error("Depth is not loaded");
				if (!depthOpaque)
					spdlog::error("Depth opaque is not loaded");
				if (depth && depthOpaque)
					*depthOpaque = *depth;

				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				RenderLayer(context, Component::RenderLayer::TT);

				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				RenderLayer(context, Component::RenderLayer::OL);
				glDisable(GL_BLEND);

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
			AssetHandle<scope<Material::Base>> m_Unlit;
		};

		class RenderToDefaultFramebuffer : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
				GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
				GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
				GL_CHECK(glDisable(GL_DEPTH_TEST));

				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

				RenderLayer(context, Component::RenderLayer::SCREEN_QUAD);

				glPolygonMode(GL_FRONT_AND_BACK, context.Debug.DrawMode);

				for (u32 i = Component::RenderLayer::SCREEN_QUAD + 1; i < Component::RenderLayer::Count; i++)
				{
					RenderLayer(context, i);
				}
			}
		};

		class Cleanup : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
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

				for (u32 i = 0; i < Component::RenderLayer::Count; i++)
				{
					context.Renderer.PerInstancedMaterial[i].clear();
					context.Renderer.PerMaterial[i].clear();
				}

				context.Debug.DebugMeshes.Tick();
			}
		};
	}
}
