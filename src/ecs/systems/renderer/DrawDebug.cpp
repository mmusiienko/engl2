#include "ecs/systems/renderer/DrawDebug.h"
#include "ui/Components.h"


namespace EnGl::System
{
	void DrawDebug::Run(EcsImpl::EntityManager& manager, GameContext& context)
	{
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
}
