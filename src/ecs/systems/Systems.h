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
				auto query1 = manager.Query<Component::Transform, Component::LocalModelMatrix>();

				for (auto [e, transform, model] : query1)
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

					transform.Dirty = false;
					transform.OnlyPosDirty = false;
				}

				auto query2 = manager.Query<Component::Transform, Component::ModelMatrix>().Exclude<Component::LocalModelMatrix>();

				for (auto [e, transform, model] : query2)
				{
					if (transform.Dirty)
					{
						UpdateModelMatrix(model.CachedModel, transform);
					}
					else if (transform.OnlyPosDirty)
					{
						UpdateModelOnlyPos(model.CachedModel, transform.Position);
					}

					transform.Dirty = false;
					transform.OnlyPosDirty = false;
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

		class SceneGraph : public SystemImpl
		{
			void CollapseMatrices(EcsImpl::EntityManager& manager, Entity e, glm::mat4& currentMatrix, bool dirty)
			{
				auto [m, lm] = manager.Get<Component::ModelMatrix, Component::LocalModelMatrix>(e);;
				dirty |= lm.Dirty;

				if (dirty)
				{
					m.CachedModel = currentMatrix * lm.CachedModel;
				}

				lm.Dirty = false;

				if (!manager.Has<Component::Children>(e))
					return;

				auto& c = manager.Get<Component::Children>(e);
				for (auto& child : c.Children)
				{
					glm::mat4 mat = m.CachedModel;
					CollapseMatrices(manager, child, mat, dirty);
				}
			}

			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto query = manager.Query<Component::ModelMatrix, Component::LocalModelMatrix, Component::Children>().Exclude<Component::Parent>();

				for (auto [e, m, lm, c] : query)
				{
					if (lm.Dirty)
					{
						m.CachedModel = lm.CachedModel;
					}

					for (auto& child : c.Children)
					{
						glm::mat4 id = m.CachedModel;
						CollapseMatrices(manager, child, id, lm.Dirty);
					}

					lm.Dirty = false;
				}

				auto query2 = manager.Query<Component::ModelMatrix, Component::LocalModelMatrix>().Exclude<Component::Children, Component::Parent>();

				for (auto [e, m, lm] : query2)
				{
					if (lm.Dirty)
						m.CachedModel = lm.CachedModel;
					lm.Dirty = false;
				}
			}
		};

		class Movement : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto query1 = manager.Query<Component::Transform, Component::PhysicalMomentum>();
				auto query2 = manager.Query<Component::Transform, Component::Velocity>();

				for (auto [e, transform, momentum] : query1)
				{
					transform.Position += context.DeltaTime * momentum.Velocity;

					transform.OnlyPosDirty = true;
				}

				for (auto [e, transform, velocity] : query2)
				{
					transform.Position += context.DeltaTime * velocity.Speed * velocity.NormalizedDirection;
					transform.OnlyPosDirty = true;
				}
			}
		};

		class ConstantRotation : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto query = manager.Query<Component::Transform, Component::ConstantRotation>();

				for (auto [e, transform, rot] : query)
				{
					transform.Rotation *= glm::normalize(glm::angleAxis(rot.Velocity.x * context.DeltaTime, Constants::RIGHT));
					transform.Rotation *= glm::normalize(glm::angleAxis(rot.Velocity.y * context.DeltaTime, Constants::UP));
					transform.Rotation *= glm::normalize(glm::angleAxis(rot.Velocity.z * context.DeltaTime, Constants::FORWARD));
					transform.Dirty = true;
				}
			}
		};

		class FollowSnap : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto query = manager.Query<Component::Transform, Component::FollowSnap>();

				for (auto [e, transform, follow] : query)
				{
					Entity toFollow = follow.Follow;

					if (manager.Has<Component::Transform>(toFollow))
					{
						const auto& toFollowTransform = manager.Get<Component::Transform>(toFollow);
						transform.Position = glm::floor(toFollowTransform.Position * glm::vec3{ follow.PosUnlock } / glm::vec3{ follow.Snap }) * glm::vec3{ follow.Snap } + follow.PosOffset;
					}
				}

			}
		};

		class Input : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto [transform, movement] = manager.Get<Component::Transform, Component::Velocity>(context.Camera.GetEntity());
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

				context.Camera.Get().Delta = movement.NormalizedDirection * context.DeltaTime * movement.Speed;
			}
		};

		class ViewMatrix : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto query = manager.Query<Component::ModelMatrix, Component::ViewMatrix>();

				for (auto [e, m, view] : query)
				{
					view.CachedView = glm::inverse(m.CachedModel);
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
						float f = 1.0f / tan(glm::radians(persp.FovDegree) * 0.5f);

						glm::mat4 projection{0.0f};


						projection[0][0] = f / persp.Aspect;
						projection[1][1] = f;
						projection[2][3] = -1.0f;
						projection[3][2] = persp.NearPlane;

						proj.CachedProjection = projection;

						persp.Dirty = false;
					}
				}

				for (auto [e, proj, ortho] : queryOrtho)
				{
					if (ortho.Dirty)
					{
						glm::mat4 mat{1.0f};

						mat[0][0] = 2.0f / (ortho.Right - ortho.Left);
						mat[1][1] = 2.0f / (ortho.Top - ortho.Bottom);
						mat[2][2] = 1.0f / (ortho.NearPlane - ortho.FarPlane);

						//mat[3][0] = -(ortho.Right + ortho.Left) / (ortho.Right - ortho.Left);
						//mat[3][1] = -(ortho.Top + ortho.Bottom) / (ortho.Top - ortho.Bottom);
						//mat[3][2] = ortho.NearPlane / (ortho.NearPlane - ortho.FarPlane);
					
						proj.CachedProjection = mat;

						ortho.Dirty = false;
					}
				}
			}
		};

		class UpdateCameras : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				for (auto& [e, camera] : context.Camera.Cameras)
				{
					auto [view, proj, m] = manager.Get<
						Component::ViewMatrix,
						Component::ProjectionMatrix,
						Component::ModelMatrix
					>(e);

					camera.InverseViewLastFrame = camera.InverseView;
					camera.InverseProjectionLastFrame = camera.InverseProjection;
					camera.InverseViewProjectionLastFrame = camera.InverseViewProjection;
					camera.ViewProjectionLastFrame = camera.ViewProjection;
					camera.ViewLastFrame = camera.View;
					camera.ProjectionLastFrame = camera.Projection;

					camera.Position = m.CachedModel[3];
					camera.Forward = -glm::normalize(m.CachedModel[2]);
					camera.View = view.CachedView;
					camera.InverseView = glm::inverse(view.CachedView);
					camera.Projection = proj.CachedProjection;
					camera.InverseProjection = glm::inverse(proj.CachedProjection);
					camera.ViewProjection = proj.CachedProjection * view.CachedView;
					camera.InverseViewProjection = glm::inverse(camera.ViewProjection);
					

					if (manager.Has<Component::PerspectiveProjection>(e))
					{
						auto persp = manager.Get<Component::PerspectiveProjection>(e);
						camera.Near = persp.NearPlane;
						camera.Far = persp.FarPlane;
						camera.Fov = glm::radians(persp.FovDegree);
						camera.Aspect = persp.Aspect;
					}
					else if (manager.Has<Component::OrthogonalProjection>(e))
					{
						auto persp = manager.Get<Component::OrthogonalProjection>(e);
						camera.Near = persp.NearPlane;
						camera.Far = persp.FarPlane;
					}
				}
			}
		};

		class UpdateCubemap : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				if (!context.Cubemap.Dirty) return;

				auto& modelC = manager.Get<Component::RenderedModel>(context.Cubemap.Id);
				auto model = AssetManager::GetAsset(modelC.Model).Asset;
				if (!model) return;

				auto mat = AssetManager::GetAsset(model->GetSubmesh(modelC.MeshIdx).Material).Asset;
				if (!mat) return;

				auto cubemapMat = dynamic_cast<Material::CubemapObj*>(mat);
				if (!cubemapMat) return;

				cubemapMat->CubemapHandle = context.Cubemap.Asset;
				context.Cubemap.Dirty = false;
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
					context.DirLight = { .Data = {.Direction = dir, .Color = d.Color}, .Rotation = t.Rotation, .Id = e };
					break;
				}

				size_t i = 0;
				for (auto [e, t, p] : queryP)
				{
					context.PointLights[i++] = { .Position = t.Position, .Color = p.Color, .Intensity = p.Intensity };
					if (context.Debug.Draw.Enabled)
					{
						static constexpr auto WIDTH = glm::vec3{ 0.1f };
						context.Debug.DebugMeshes.FrameCube(t.Position - WIDTH, t.Position + WIDTH);
					}

					if (i >= Shader::MAX_LIGHTS) break;
				}
			}

			f32 m_DirLightCameraDist = 800.0f;
		};

		class PrepareDebugDraw : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{ 
				if (context.Debug.Draw.Enabled && context.Debug.Draw.Camera)
				{
					static const glm::vec4 tln{ -1.0f, 1.0f, 1.0f, 1.0f };
					static const glm::vec4 bln{ -1.0f, -1.0f, 1.0f, 1.0f };
					static const glm::vec4 trn{ 1.0f, 1.0f, 1.0f, 1.0f };
					static const glm::vec4 brn{ 1.0f, -1.0f, 1.0f, 1.0f };
					static const f32 farDistDebug = 50.0f;
					static const f32 forwardDistDebug = farDistDebug * 2.0f;

					for (const auto& [e, camera] : context.Camera.Cameras)
					{
						if (e == context.Camera.GetEntity()) continue;

						context.Debug.DebugMeshes.Line(camera.Position, camera.Position + forwardDistDebug * camera.Forward, DebugMesh::DEFAULT_COLOR, 0.1f);

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
						context.Debug.DebugMeshes.Line(camera.Position, ftl, DebugMesh::DEFAULT_COLOR, 0.1f);
						context.Debug.DebugMeshes.Line(camera.Position, ftr, DebugMesh::DEFAULT_COLOR, 0.1f);
						context.Debug.DebugMeshes.Line(camera.Position, fbl, DebugMesh::DEFAULT_COLOR, 0.1f);
						context.Debug.DebugMeshes.Line(camera.Position, fbr, DebugMesh::DEFAULT_COLOR, 0.1f);

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
							AddToMaterialMap(e, context, model, i, transform, modelComp.Layer);
						}
					}
					else
					{
						AddToMaterialMap(e, context, model, modelComp.MeshIdx, transform, modelComp.Layer);
					}
				}
			}

			static void AddToMaterialMap(
				Entity entity,
				GameContext& context,
				Model* model,
				u32 idx,
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

					auto& bucket = map[material->Get().Id];
					bucket.InstanceDatas.push_back(
						{
							.Mesh = submesh.Mesh,
							.Material = submesh.Material,
							.Data =
							{
								.Model = modelMatrix.CachedModel,
								.Normal = glm::transpose(glm::inverse(modelMatrix.CachedModel))
							},
							.Entity= entity
						}
					);
				}
			}
		};

		static void RenderLayer(EcsImpl::EntityManager& manager, GameContext& context, u32 layer, bool isUnlit)
		{

			for (auto& [key, value] : context.Renderer.PerMaterial[layer])
			{
				auto material = AssetManager::GetAsset(value.InstanceDatas[0].Material).Asset;

				if (!material) continue;

				auto shaderH = isUnlit ? material->GetUnlit() : material->Get();
				auto shader = AssetManager::GetAsset(shaderH).Asset;
				if (!shader) continue;

				isUnlit ? material->SetCommonUniformsUnlit(shader, context) : material->SetCommonUniforms(shader, context);

				for (auto& data : value.InstanceDatas)
				{
					auto mesh = AssetManager::GetAssetNoCheck(data.Mesh);

					auto matInstance = AssetManager::GetAsset(data.Material).Asset;

					if (!matInstance) continue;

					isUnlit ? matInstance->SetModelUnlit(shader, data.Data.Model, data.Data.Normal) : matInstance->SetModel(shader, data.Data.Model, data.Data.Normal);
					isUnlit ? matInstance->SetUniformsUnlit(shader) : matInstance->SetUniforms(shader);

					if (manager.Has<Component::AnimationData>(data.Entity))
					{
						auto ad = manager.Get<Component::AnimationData>(data.Entity);
						auto skelInstance = AssetManager::GetAsset(ad.Skeleton).Asset;
						if (!skelInstance) continue;

						auto ssbo = AssetManager::GetAsset(skelInstance->BoneTransforms).Asset;
						if (!ssbo) continue;
						ssbo->Bind(0);
					}

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

			//TODO: support instancing
			for (auto& [key, value] : context.Renderer.PerInstancedMaterial[layer])
			{
				auto mesh = AssetManager::GetAsset(key.MeshHandle).Asset;
				auto material = AssetManager::GetAsset(key.MaterialHandle).Asset;

				if (!mesh || !material) continue;

				auto shaderH = material->Get();
				auto shader = AssetManager::GetAsset(shaderH).Asset;

				if (!shader) continue;

				material->SetCommonUniforms(shader, context);
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

				material->SetUniforms(shader);
				mesh->DrawInstanced();
			}
		}

		class RenderToFramebuffer : public SystemImpl
		{
			void Init(EcsImpl::EntityManager& manager)
			{
				auto unlit = make_scope<Material::Unlit>();
				unlit->Color = {};
				m_Unlit = AssetManager::PutScope<Material::Base>(
					std::move(unlit)
				);

				m_ShadowCombined = AssetManager::Put<Texture2D>(1u, 1u, Texture::CreationInfoFromData{
						.CpuFormat = GL_RGBA,
						.GpuFormat = GL_RGBA32F,
						.Common = {.MinFilter = GL_NEAREST, .MagFilter = GL_NEAREST}
				});
			}

			void RenderShadows(EcsImpl::EntityManager& manager, GameContext& context)
			{
				glEnable(GL_DEPTH_CLAMP);
				glClearDepth(0.0);
				glDepthFunc(GL_GREATER);
				glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(context.Renderer.CascadedShadowMap.PolygonOffset.x, context.Renderer.CascadedShadowMap.PolygonOffset.y);

				Entity prevCamera = context.Camera.GetEntity();
				for (u32 i = 0; i < GameContext::RendererInfo::CascadedShadowMapInfo::NShadowMapCascades; i++)
				{
					context.Renderer.CascadedShadowMap.DirShadowFramebuffer[i]->Bind();

					glClear(GL_DEPTH_BUFFER_BIT);
					glDrawBuffer(GL_NONE);
					glReadBuffer(GL_NONE);
					context.Camera.SetCamera(context.Renderer.CascadedShadowMap.CascadeCamera[i]);

					RenderLayer(manager, context, Component::RenderLayer::OQ, true);

					//glEnable(GL_BLEND);
					//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					//RenderLayer(context, Component::RenderLayer::TT, true);
					//glDisable(GL_BLEND);
				}
				context.Camera.SetCamera(prevCamera);

				glDisable(GL_DEPTH_CLAMP);
				glDisable(GL_POLYGON_OFFSET_FILL);

				DispatchCSMCombine(manager, context);

				context.Renderer.CascadedShadowMap.ShadowMap = m_ShadowCombined;
			}

			void DispatchCSMCombine(EcsImpl::EntityManager& manager, GameContext& context)
			{
				auto shader = AssetManager::GetAsset(m_Combine).Asset;
				if (!shader) return;

				auto texture = AssetManager::GetAsset(m_ShadowCombined).Asset;
				if (!texture)
				{
					return;
				}

				if (context.Framebuffer.MainFramebuffer->Resolution() != m_Res)
				{
					m_Res = context.Framebuffer.MainFramebuffer->Resolution();
					texture->Properties().w = m_Res.x;
					texture->Properties().h = m_Res.y;
					texture->Update();
				}

				shader->Use();

				shader->SetUniform("uInvView", context.Camera.Get().InverseView);
				shader->SetUniform("uInvProjection", context.Camera.Get().InverseProjection);
				shader->SetUniform("uResolution", m_Res);

				shader->BindWriteTexture(*texture, 0u);

				auto depth = AssetManager::GetAsset(context.Framebuffer.MainFramebuffer->Depth()).Asset;

				if (!depth)
				{
					spdlog::error("Depth is not loaded");
					return;
				}

				shader->SetUniform("uDepth", *depth, 1u);
				shader->SetUniform("uNear", context.Camera.Get().Near);

				for (u32 i = 0; i < GameContext::RendererInfo::CascadedShadowMapInfo::NShadowMapCascades; i++)
				{

					auto fb = context.Renderer.CascadedShadowMap.DirShadowFramebuffer[i];
					auto e = context.Renderer.CascadedShadowMap.CascadeCamera[i];
					auto depth = AssetManager::GetAsset(fb->Depth()).Asset;

					if (!depth) { spdlog::error("Shadow framebuffer depth is not loaded"); continue; }

					auto [view, proj] = manager.Get<Component::ViewMatrix, Component::ProjectionMatrix>(e);

					auto formatted = std::format("uCascades[{}].", i);
					shader->SetUniform(formatted + "LightViewProjection", proj.CachedProjection * view.CachedView);
					shader->SetUniform(formatted + "ShadowMap", *depth, 2u + i);
					shader->SetUniform(formatted + "Split", context.Renderer.CascadedShadowMap.DepthSplit[i]);
				}

				shader->DispatchWait(
					ComputeShader::ComputeInfo{.GroupSizeX = static_cast<u32> (glm::ceil(m_Res.x / 16.0f)), .GroupSizeY = static_cast<u32> (glm::ceil(m_Res.y / 16.0f)) },
					GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT
				);
			}

			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				
				glPolygonMode(GL_FRONT_AND_BACK, context.Debug.DrawMode);
				glDisable(GL_BLEND);
				glDepthFunc(GL_GREATER);
				glEnable(GL_DEPTH_TEST);

				context.Framebuffer.MainFramebuffer->Bind();
				glClear(GL_DEPTH_BUFFER_BIT);
				glClear(GL_COLOR_BUFFER_BIT);
				glDrawBuffer(GL_NONE);
				glReadBuffer(GL_NONE);
				RenderLayer(manager, context, Component::RenderLayer::OQ, true);

				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				RenderLayer(manager, context, Component::RenderLayer::TT, true);
				glDisable(GL_BLEND);

				RenderShadows(manager, context);

				context.Framebuffer.MainFramebuffer->Bind();
				glDepthFunc(GL_GEQUAL);
				glDrawBuffer(GL_COLOR_ATTACHMENT0);
				glReadBuffer(GL_COLOR_ATTACHMENT0);
				RenderLayer(manager, context, Component::RenderLayer::OQ, false);
				
				glEnable(GL_DEPTH_TEST);
				glDepthMask(GL_FALSE);
				glCullFace(GL_FRONT);

				RenderLayer(manager, context, Component::RenderLayer::CUBEMAP, false);
				glDepthMask(GL_TRUE);
				glCullFace(GL_BACK);

				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				RenderLayer(manager, context, Component::RenderLayer::TT, false);

				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				RenderLayer(manager, context, Component::RenderLayer::OL, false);
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
			AssetHandle<ComputeShader> m_Combine = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "CSMCombine");

			AssetHandle<Texture2D> m_ShadowCombined;

			glm::uvec2 m_Res{ 0u };

			AssetHandle<Material::Base> m_Unlit;
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

				RenderLayer(manager, context, Component::RenderLayer::SCREEN_QUAD, false);

				glPolygonMode(GL_FRONT_AND_BACK, context.Debug.DrawMode);

				for (u32 i = Component::RenderLayer::SCREEN_QUAD + 1; i < Component::RenderLayer::Count; i++)
				{
					RenderLayer(manager, context, i, false);
				}
			}
		};

		class Cleanup : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto queryModelMatrix = manager.Query<Component::ModelMatrix>();

				for (u32 i = 0; i < Component::RenderLayer::Count; i++)
				{
					context.Renderer.PerInstancedMaterial[i].clear();
					context.Renderer.PerMaterial[i].clear();
				}

				context.Debug.DebugMeshes.Tick();
			}
		};

		class Gravity : public SystemImpl
		{
			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto query = manager.Query<Component::PhysicalMomentum>();

				for (auto [e, p] : query)
				{
					if (p.InverseMass != 0.0f)
					{
						p.Velocity -= 9.81f * context.DeltaTime;
					}
				}
			}
		};

		class CollisionDetection : public SystemImpl
		{
			struct Contact
			{
				glm::vec3 Normal;
				f32 Depth;
			};

			struct Body
			{
				glm::vec3* Pos;
				Component::PhysicalMomentum* P;
			};

			struct Collision 
			{
				Contact Contact;
				Body Body1;
				Body Body2;
				f32 Compliance;
				bool IsConstraint;
			};

			void Run(EcsImpl::EntityManager& manager, GameContext& context) override
			{
				auto query = manager.Query<Component::Transform, Component::SphereCollider, Component::PhysicalMomentum>();
				auto queryConstraint = manager.Query<Component::LengthConstraint>();
				using collider = std::tuple<glm::vec3*, Component::SphereCollider*, Component::PhysicalMomentum*>;
				using collisionEl = std::tuple<glm::vec3*, Contact, Component::PhysicalMomentum*>;
				std::vector<collider> all;
				std::vector<Collision> collisions;
				all.reserve(manager.CountAll<Component::Transform, Component::SphereCollider, Component::PhysicalMomentum>());

				for (auto [e, transform, collider, pm] : query)
				{
					all.push_back({ &transform.Position, &collider, &pm });
				}

				for (size_t i = 0; i < all.size(); i++)
				{
					for (size_t j = i + 1; j < all.size(); j++)
					{
						auto& [pos1, sphere1, p1] = all[i];
						auto& [pos2, sphere2, p2] = all[j];
						
						glm::vec3 diff = *pos2 + sphere2->Offset - *pos1 - sphere1->Offset;
						f32 dist = glm::length(diff);
						f32 depth = sphere1->Radius + sphere2->Radius - dist;
						if (dist > Constants::TOLERANCE && depth >= Constants::TOLERANCE)
						{
							Collision col{};
							col.Body1.Pos = pos1;
							col.Body2.Pos = pos2;
							col.Body1.P = p1;
							col.Body2.P = p2;
							col.Compliance = std::min(p1->Restitution, p2->Restitution);
							col.Contact.Depth = depth;
							col.Contact.Normal = diff / dist;
							col.IsConstraint = false;
							collisions.emplace_back(std::move(col));
						}
					}
				}

				for (auto [e, constraint] : queryConstraint)
				{
					if (constraint.E1 == 0 || constraint.E2 == 0) continue;

					auto [t1, c1, p1] = manager.Get<Component::Transform, Component::SphereCollider, Component::PhysicalMomentum>(constraint.E1);
					auto [t2, c2, p2] = manager.Get<Component::Transform, Component::SphereCollider, Component::PhysicalMomentum>(constraint.E2);

					glm::vec3 diff = t2.Position + c2.Offset - t1.Position - c1.Offset;
					f32 dist = glm::length(diff);
					f32 depth = dist - constraint.Length;
					if (dist > Constants::TOLERANCE && abs(depth) >= Constants::TOLERANCE)
					{
						Collision col{};
						col.Body1.Pos = &t1.Position;
						col.Body2.Pos = &t2.Position;
						col.Body1.P = &p1;
						col.Body2.P = &p2;
						col.Contact.Normal = -diff / dist;
						col.Contact.Depth = depth;
						col.Compliance = 0.0f;
						col.IsConstraint = true;

						collisions.emplace_back(std::move(col));
					}
				}

				for (const auto& col : collisions)
				{
					f32 relativeVel = glm::dot(col.Contact.Normal, col.Body2.P->Velocity - col.Body1.P->Velocity);

					if (!col.IsConstraint && relativeVel > 0.0f) continue;

					f32 invsum = col.Body1.P->InverseMass + col.Body2.P->InverseMass;

					f32 j = -(1 + col.Compliance) * relativeVel / invsum;

					col.Body1.P->Velocity -= j * col.Body1.P->InverseMass * col.Contact.Normal;
					col.Body2.P->Velocity += j * col.Body2.P->InverseMass * col.Contact.Normal;

					f32 cor =  !col.IsConstraint ? std::max(col.Contact.Depth - Constants::TOLERANCE, 0.0f) / invsum : col.Contact.Depth;
					glm::vec3 corV = cor * col.Contact.Normal;

					*col.Body1.Pos -= col.Body1.P->InverseMass * corV;
					*col.Body2.Pos += col.Body2.P->InverseMass * corV;
				}
			}
		};
	}
}
