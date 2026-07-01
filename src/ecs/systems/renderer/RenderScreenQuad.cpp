#include "ecs/systems/renderer/RenderScreenQuad.h"
#include "ecs/systems/renderer/Common.h"
#include "ui/Components.h"


namespace EnGl::System
{
	void RenderScreenQuad::Run(EcsImpl::EntityManager& manager, GameContext& context)
	{
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 1.0f));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
		GL_CHECK(glDisable(GL_DEPTH_TEST));

		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		Renderer::Common::RenderLayer(manager, context, Component::RenderLayer::SCREEN_QUAD, RenderpassType::NORMAL);

		glPolygonMode(GL_FRONT_AND_BACK, context.Debug.DrawMode);

		for (u32 i = Component::RenderLayer::SCREEN_QUAD + 1; i < Component::RenderLayer::Count; i++)
		{
			Renderer::Common::RenderLayer(manager, context, i, RenderpassType::NORMAL);
		}
	}
}
