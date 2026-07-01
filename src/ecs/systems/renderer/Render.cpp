#include "ecs/systems/renderer/Render.h"
#include "ecs/systems/renderer/Common.h"
#include "ui/Components.h"


namespace EnGl::System
{
	void Render::Run(EcsImpl::EntityManager& manager, GameContext& context)
	{
		context.Framebuffer.MainFramebuffer->Bind();

		glDepthFunc(GL_EQUAL);
		u32 buffers[] = { GL_COLOR_ATTACHMENT0, GL_NONE };
		glDrawBuffers(2, buffers);
		glDepthMask(GL_FALSE);

		Renderer::Common::RenderLayer(manager, context, Component::RenderLayer::OQ);

		glCullFace(GL_FRONT);
		Renderer::Common::RenderLayer(manager, context, Component::RenderLayer::CUBEMAP);
		glCullFace(GL_BACK);

		glDepthFunc(GL_GEQUAL);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Renderer::Common::RenderLayer(manager, context, Component::RenderLayer::TT);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		Renderer::Common::RenderLayer(manager, context, Component::RenderLayer::OL);
		glDisable(GL_BLEND);
	}

	void Render::Editor(EcsImpl::EntityManager& manager, GameContext& context)
	{
		UiComponents::Texture2DView(context.Framebuffer.MainFramebuffer->Color()[0], glm::vec2{ 0.1f });
		UiComponents::Texture2DView(context.Framebuffer.MainFramebuffer->Color()[1], glm::vec2{ 0.1f });
		UiComponents::Texture2DView(context.Framebuffer.MainFramebuffer->Depth(), glm::vec2{ 0.1f });
	}
}
