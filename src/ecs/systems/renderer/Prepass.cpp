#include "ecs/systems/renderer/Prepass.h"
#include "ui/Components.h"
#include "ecs/systems/renderer/Common.h"


namespace EnGl::System
{
	void Prepass::Run(EcsImpl::EntityManager& manager, GameContext& context)
	{
		static const AssetHandle<ComputeShader> copyDepthHandle = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "DepthResolveMS");

		context.Framebuffer.FramebufferMS->Bind();

		u32 buffersPre[] = { GL_NONE, GL_COLOR_ATTACHMENT1 };
		GL_CHECK(glDrawBuffers(2, buffersPre));
		Renderer::Common::RenderLayer(manager, context, Component::RenderLayer::OQ, RenderpassType::PREPASS);

		auto depth = AssetManager::GetAsset(context.Framebuffer.FramebufferMS->Depth()).Asset;
		auto depthWithoutT = AssetManager::GetAsset(m_DepthWithoutTransparents).Asset;
		auto shader = AssetManager::GetAsset(copyDepthHandle).Asset;
		if (!depth || !depthWithoutT || !shader)
			return;
		
		if (context.Framebuffer.FramebufferMS->Resolution() != m_Res)
		{
			m_Res = context.Framebuffer.FramebufferMS->Resolution();
			depthWithoutT->Properties().w = m_Res.x;
			depthWithoutT->Properties().h = m_Res.y;
			depthWithoutT->Update();
		}

		shader->Use();
		shader->SetUniform("uSource", *depth, 0u);
		shader->SetUniform("uNumSamples", depth->Properties().NumSamples);
		shader->BindWriteTexture(*depthWithoutT, 1u);
		u32 w = context.Framebuffer.MainFramebuffer->Resolution().x;
		u32 h = context.Framebuffer.MainFramebuffer->Resolution().y;
		shader->DispatchWait(
			ComputeShader::ComputeInfo{ .GroupSizeX = static_cast<u32> (glm::ceil(w / 16.0f)), .GroupSizeY = static_cast<u32> (glm::ceil(h / 16.0f))},
			GL_TEXTURE_FETCH_BARRIER_BIT
		);
		context.Renderer.DepthWithoutTransparents = m_DepthWithoutTransparents;

		GL_CHECK(glEnable(GL_BLEND));
		GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		Renderer::Common::RenderLayer(manager, context, Component::RenderLayer::TT, RenderpassType::SHADOW);
		GL_CHECK(glDisable(GL_BLEND));

		GL_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, context.Framebuffer.FramebufferMS->Id()));
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, context.Framebuffer.MainFramebuffer->Id()));
		GL_CHECK(glReadBuffer(GL_COLOR_ATTACHMENT1));
		GL_CHECK(glDrawBuffers(2, buffersPre));
		GL_CHECK(glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST));
	}

	void Prepass::Init(EcsImpl::EntityManager& manager)
	{
		m_DepthWithoutTransparents = AssetManager::Put<Texture2D>(
			Texture2D{
				1u, 1u,
				Texture::CreationInfoFromData{
					.CpuFormat = GL_RED,
					.GpuFormat = GL_R32F,
					.DataType = GL_FLOAT
				}
			}
		);
	}

	void Prepass::Editor(EcsImpl::EntityManager& manager, GameContext& context)
	{
		UiComponents::Texture2DView(context.Framebuffer.MainFramebuffer->Color()[0], glm::vec2{ 0.1f });
		UiComponents::Texture2DView(context.Framebuffer.MainFramebuffer->Color()[1], glm::vec2{ 0.1f });
		UiComponents::Texture2DView(context.Renderer.DepthWithoutTransparents, glm::vec2{ 0.1f });
	}
}
