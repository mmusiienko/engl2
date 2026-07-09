#include "ecs/systems/renderer/Render.h"
#include "ecs/systems/renderer/Common.h"
#include "ui/Components.h"


namespace EnGl::System
{
	void Render::Run(EcsImpl::EntityManager& manager, GameContext& context)
	{
		context.Framebuffer.FramebufferMS->Bind();

		GL_CHECK(glDepthFunc(GL_EQUAL));
		u32 buffers[] = { GL_COLOR_ATTACHMENT0, GL_NONE };
		GL_CHECK(glDrawBuffers(2, buffers));
		GL_CHECK(glDepthMask(GL_FALSE));

		Renderer::Common::RenderLayer(manager, context, Component::RenderLayer::OQ);

		GL_CHECK(glCullFace(GL_FRONT));
		Renderer::Common::RenderLayer(manager, context, Component::RenderLayer::CUBEMAP);
		GL_CHECK(glCullFace(GL_BACK));

		GL_CHECK(glDepthFunc(GL_GEQUAL));
		GL_CHECK(glEnable(GL_BLEND));
		GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		Renderer::Common::RenderLayer(manager, context, Component::RenderLayer::TT);

		GL_CHECK(glBlendFunc(GL_SRC_ALPHA, GL_ONE));
		Renderer::Common::RenderLayer(manager, context, Component::RenderLayer::OL);
		GL_CHECK(glDisable(GL_BLEND));

		u32 w = context.Framebuffer.MainFramebuffer->Resolution().x;
		u32 h = context.Framebuffer.MainFramebuffer->Resolution().y;
		GL_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, context.Framebuffer.FramebufferMS->Id()));
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, context.Framebuffer.MainFramebuffer->Id()));
		GL_CHECK(glReadBuffer(GL_COLOR_ATTACHMENT0));
		GL_CHECK(glDrawBuffers(2, buffers));
		GL_CHECK(glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST));
	}

	void Render::Editor(EcsImpl::EntityManager& manager, GameContext& context)
	{
		UiComponents::Texture2DView(context.Framebuffer.MainFramebuffer->Color()[0], glm::vec2{ 0.1f });
		UiComponents::Texture2DView(context.Framebuffer.MainFramebuffer->Color()[1], glm::vec2{ 0.1f });
		UiComponents::Texture2DView(context.Framebuffer.MainFramebuffer->Depth(), glm::vec2{ 0.1f });
	}
}
