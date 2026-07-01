#include "ecs/systems/renderer/Prepass.h"
#include "ui/Components.h"
#include "ecs/systems/renderer/Common.h"


namespace EnGl::System
{
	void Prepass::Run(EcsImpl::EntityManager& manager, GameContext& context)
	{
		u32 buffersPre[] = { GL_NONE, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, buffersPre);
		Renderer::Common::RenderLayer(manager, context, Component::RenderLayer::OQ, RenderpassType::PREPASS);

		auto depth = AssetManager::GetAsset(context.Framebuffer.MainFramebuffer->Depth()).Asset;
		auto depthWithoutT = AssetManager::GetAsset(context.Renderer.DepthWithoutTransparents).Asset;
		if (!depth || !depthWithoutT) return;
		*depthWithoutT = *depth;

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Renderer::Common::RenderLayer(manager, context, Component::RenderLayer::TT, RenderpassType::SHADOW);
		glDisable(GL_BLEND);
	}

	void Prepass::Editor(EcsImpl::EntityManager& manager, GameContext& context)
	{
		UiComponents::Texture2DView(context.Framebuffer.MainFramebuffer->Color()[0], glm::vec2{ 0.1f });
		UiComponents::Texture2DView(context.Framebuffer.MainFramebuffer->Color()[1], glm::vec2{ 0.1f });
		UiComponents::Texture2DView(context.Renderer.DepthWithoutTransparents, glm::vec2{ 0.1f });
	}
}
