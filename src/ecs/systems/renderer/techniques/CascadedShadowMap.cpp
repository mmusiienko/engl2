#include "ecs/systems/renderer/techniques/CascadedShadowMap.h"
#include "ecs/systems/renderer/Common.h"
#include "ui/Components.h"
#include "algorithm/compute/Blur.h"


namespace EnGl::System
{
	static constexpr std::array<glm::vec2, 4> ndcFrustrumCoords
	{
		glm::vec2{-1.0f, -1.0f},
		glm::vec2{-1.0f, 1.0f},
		glm::vec2{1.0f, 1.0f},
		glm::vec2{1.0f, -1.0f}
	};

	static std::array<glm::vec4, 8> GetTransformedCorners(GameContext& context, f32 z1, f32 z2)
	{
		std::array<glm::vec4, 8> res;
		u32 i = 0;

		for (u32 z = 0; z <= 1; z++)
		{
			for (const auto& corner : ndcFrustrumCoords)
			{
				auto transformedVec = context.Camera.GetTarget().InverseViewProjection * glm::vec4{corner.x ,corner.y, z1 * (1 - z) + z2 * z, 1.0f};
				res[i] = transformedVec / transformedVec.w;
				i++;
			}
		}

		return res;
	}
			

	void CascadedShadowMapFit::Run(EcsImpl::EntityManager & manager, GameContext & context)
	{
		auto& camInfo = context.Camera.GetTarget();
		auto e = context.Camera.GetEntity();

		f32 z1 = 1.0f;

		for (u32 i = 0; i < GameContext::RendererInfo::CascadedShadowMapInfo::NShadowMapCascades; i++)
		{
			if (e == context.Renderer.CascadedShadowMap.CascadeCamera[i]) continue;
			if (camInfo.Near == 0.0f) continue;

			f32 z2 = camInfo.Near / context.Renderer.CascadedShadowMap.DepthSplit[i];
			auto corners = GetTransformedCorners(context, z1, z2);

			Entity cam = context.Renderer.CascadedShadowMap.CascadeCamera[i];
			auto& cascadeCamInfo = context.Camera.Cameras.at(cam);

			auto [camTransform, view, ortho, proj] = manager.Get<Component::Transform, Component::ViewMatrix, Component::OrthogonalProjection, Component::ProjectionMatrix>(cam);

            f32 dk = glm::ceil(glm::max(glm::length(corners[6] - corners[0]), glm::length(corners[6] - corners[4])));
            f32 t = dk / static_cast<f32>(context.Renderer.CascadedShadowMap.TextureSize[i]);
            f32 halfdk = dk * 0.5f;

            glm::mat4 tempview = glm::lookAt(glm::vec3{}, cascadeCamInfo.Forward, Constants::UP);
            glm::vec3 min = glm::vec3{ std::numeric_limits<f32>::max() };
            glm::vec3 max = glm::vec3{ std::numeric_limits<f32>::lowest() };

            for (auto& c : corners)
            {
                auto corner = tempview * c;
                min.x = glm::min(corner.x, min.x);
                min.y = glm::min(corner.y, min.y);
                min.z = glm::min(corner.z, min.z);

                max.x = glm::max(corner.x, max.x);
                max.y = glm::max(corner.y, max.y);
                max.z = glm::max(corner.z, max.z);
            }

            auto inv = glm::inverse(tempview);
            glm::vec3 sk = inv *
                glm::vec4{
                    glm::floor((min.x + max.x) * 0.5f / t) * t,
                    glm::floor((min.y + max.y) * 0.5f / t) * t,
                    min.z,
                    1.0f
            };

            glm::vec3 pos = sk + 0.01f * context.DirLight.Data.Direction;

            view.CachedView = glm::lookAt(pos, sk, Constants::UP);

            //if (context.Debug.Draw.Enabled)
            //{
            //    context.Debug.DebugMeshes.FrameCube(pos + glm::vec3{ -10.0f }, pos + glm::vec3{ 10.0f }, glm::vec4{0.0f, 1.0f, 0.0f, 1.0f});
            //    context.Debug.DebugMeshes.FrameCube(sk + glm::vec3{ -10.0f }, sk + glm::vec3{ 10.0f });
            //}

            camTransform.Position = pos;
            camTransform.Rotation = context.DirLight.Rotation;
            camTransform.Dirty = true;

            ortho.Left = -halfdk;
            ortho.Right = halfdk;
            ortho.Bottom = -halfdk;
            ortho.Top = halfdk;
			
            ortho.NearPlane = min.z;
            ortho.FarPlane = max.z;
            ortho.Dirty = false;

            glm::mat4 mat{ 1.0f };

            mat[0][0] = 2.0f / dk;
            mat[1][1] = 2.0f / dk;
            mat[2][2] = 1.0f / (ortho.FarPlane - ortho.NearPlane);

            proj.CachedProjection = mat;

            cascadeCamInfo.View = view.CachedView;
            cascadeCamInfo.Projection = proj.CachedProjection;
            cascadeCamInfo.InverseView = glm::inverse(view.CachedView);
            cascadeCamInfo.InverseProjection = glm::inverse(proj.CachedProjection);
            cascadeCamInfo.InverseViewProjection = cascadeCamInfo.InverseProjection * cascadeCamInfo.InverseView;
            cascadeCamInfo.ViewProjection = proj.CachedProjection * view.CachedView;

            cascadeCamInfo.Position = camTransform.Position;
            cascadeCamInfo.Forward = camTransform.Rotation * Constants::FORWARD;
            cascadeCamInfo.Near = ortho.NearPlane;
            cascadeCamInfo.Far = ortho.FarPlane;

            z1 = z2;
		}
	}

	void System::CascadedShadowMapFit::Editor(EcsImpl::EntityManager& manager, GameContext& context)
	{
		ImGui::InputFloat2("PolygonOffset", glm::value_ptr(context.Renderer.CascadedShadowMap.PolygonOffset));
		for (u32 i = 0; i < GameContext::RendererInfo::CascadedShadowMapInfo::NShadowMapCascades; i++)
		{
			UiComponents::Texture2DView(context.Renderer.CascadedShadowMap.DirShadowFramebuffer[i]->Depth(), glm::vec2{0.1f});
		}
	}

	void System::CascadedShadowMapRender::RenderShadows(EcsImpl::EntityManager& manager, GameContext& context)
	{
		GL_CHECK(glEnable(GL_DEPTH_CLAMP));
		GL_CHECK(glClearDepth(0.0));
		GL_CHECK(glDepthFunc(GL_GREATER));
		GL_CHECK(glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE));
		GL_CHECK(glEnable(GL_POLYGON_OFFSET_FILL));
		GL_CHECK(glPolygonOffset(context.Renderer.CascadedShadowMap.PolygonOffset.x, context.Renderer.CascadedShadowMap.PolygonOffset.y));

		Entity prevCamera = context.Camera.GetEntity();
		for (u32 i = 0; i < GameContext::RendererInfo::CascadedShadowMapInfo::NShadowMapCascades; i++)
		{
			context.Renderer.CascadedShadowMap.DirShadowFramebuffer[i]->Bind();
			GL_CHECK(glDrawBuffer(GL_NONE));
			GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT));
			context.Camera.SetCamera(context.Renderer.CascadedShadowMap.CascadeCamera[i]);

			Renderer::Common::RenderLayer(manager, context, Component::RenderLayer::OQ, RenderpassType::SHADOW);
		}
		context.Camera.SetCamera(prevCamera);

		GL_CHECK(glDisable(GL_DEPTH_CLAMP));
		GL_CHECK(glDisable(GL_POLYGON_OFFSET_FILL));

		DispatchCSMCombine(manager, context);
	}

	void System::CascadedShadowMapRender::DispatchCSMCombine(EcsImpl::EntityManager& manager, GameContext& context)
	{
		AssetHandle<ComputeShader> m_Combine = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "CSMCombine");
		auto shader = AssetManager::GetAsset(m_Combine).Asset;
		if (!shader) return;

		auto texture = AssetManager::GetAsset(m_ShadowCombined).Asset;
		auto texture2 = AssetManager::GetAsset(m_ShadowCombined2).Asset;
		if (!texture || !texture2)
			return;

		if (context.Framebuffer.MainFramebuffer->Resolution() != m_Res)
		{
			m_Res = context.Framebuffer.MainFramebuffer->Resolution();
			texture->Properties().w = m_Res.x;
			texture->Properties().h = m_Res.y;
			texture->Update();

			texture2->Properties().w = m_Res.x;
			texture2->Properties().h = m_Res.y;
			texture2->Update();
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
			ComputeShader::ComputeInfo{ .GroupSizeX = static_cast<u32> (glm::ceil(m_Res.x / 16.0f)), .GroupSizeY = static_cast<u32> (glm::ceil(m_Res.y / 16.0f)) },
			GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT
		);

		

		Compute::Blur(texture, texture2, {.Depth = depth, .NearPlane = context.Camera.Get().Near, .DepthReject = m_DepthRejectBlur });
	}

	void System::CascadedShadowMapRender::Init(EcsImpl::EntityManager& manager)
	{
		m_ShadowCombined = AssetManager::Put<Texture2D>(1u, 1u, Texture::CreationInfoFromData{
			.CpuFormat = GL_RED,
			.GpuFormat = GL_R8,
			.Common = {.MinFilter = GL_NEAREST, .MagFilter = GL_NEAREST}
		});

		m_ShadowCombined2 = AssetManager::Put<Texture2D>(1u, 1u, Texture::CreationInfoFromData{
			.CpuFormat = GL_RED,
			.GpuFormat = GL_R8,
			.Common = {.MinFilter = GL_NEAREST, .MagFilter = GL_NEAREST}
		});
	}

	void System::CascadedShadowMapRender::Run(EcsImpl::EntityManager& manager, GameContext& context)
    {
		RenderShadows(manager, context);
		context.Renderer.CascadedShadowMap.ShadowMap = m_ShadowCombined;
    }

    void System::CascadedShadowMapRender::Editor(EcsImpl::EntityManager& manager, GameContext& context)
    {
		ImGui::InputFloat2("PolygonOffset", glm::value_ptr(context.Renderer.CascadedShadowMap.PolygonOffset));
		ImGui::InputFloat("DepthRejectBlur", &m_DepthRejectBlur);
        UiComponents::Texture2DView(m_ShadowCombined, glm::vec2{ 0.1f });
    }
}