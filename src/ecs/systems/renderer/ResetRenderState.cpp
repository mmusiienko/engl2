#include "ecs/systems/renderer/ResetRenderState.h"
#include "ui/Components.h"


namespace EnGl::System
{
	void ResetRenderState::Run(EcsImpl::EntityManager& manager, GameContext& context)
	{
		glPolygonMode(GL_FRONT_AND_BACK, context.Debug.DrawMode);
		glDisable(GL_BLEND);
		glDepthFunc(GL_GREATER);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);

		context.Framebuffer.MainFramebuffer->Bind();
		glClear(GL_DEPTH_BUFFER_BIT);
		GLenum buffersClear[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, buffersClear);
		glClear(GL_COLOR_BUFFER_BIT);
	}
}
