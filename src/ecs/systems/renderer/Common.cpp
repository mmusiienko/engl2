#include "ecs/systems/renderer/Common.h"


namespace EnGl::System
{
	void Renderer::Common::RenderLayer(EcsImpl::EntityManager& manager, GameContext& context, u32 layer, RenderpassType type)
	{
		for (auto& [key, value] : context.Renderer.PerMaterial[layer])
		{
			auto material = AssetManager::GetAsset(value.InstanceDatas[0].Material).Asset;

			if (!material) continue;

			AssetHandle<Shader> shaderH = material->Get(type);

			auto shader = AssetManager::GetAsset(shaderH).Asset;
			if (!shader) continue;

			material->SetCommonUniforms(shader, context, type);

			for (auto& data : value.InstanceDatas)
			{
				auto mesh = AssetManager::GetAssetNoCheck(data.Mesh);

				auto matInstance = AssetManager::GetAsset(data.Material).Asset;

				if (!matInstance) continue;

				matInstance->SetMatrices(shader, data.Data.Model, data.Data.Normal, type);
				matInstance->SetUniforms(shader, type);

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
		//for (auto& [key, value] : context.Renderer.PerInstancedMaterial[layer])
		//{
		//	auto mesh = AssetManager::GetAsset(key.MeshHandle).Asset;
		//	auto material = AssetManager::GetAsset(key.MaterialHandle).Asset;

		//	if (!mesh || !material) continue;

		//	auto shaderH = material->Get();
		//	auto shader = AssetManager::GetAsset(shaderH).Asset;

		//	if (!shader) continue;

		//	material->SetCommonUniforms(shader, context);
		//	mesh->UpdateInstanceBuffer(value.Data);

		//	if (context.Debug.Draw.Enabled && context.Debug.Draw.AABB)
		//	{
		//		auto aabb = mesh->GetAABB();
		//		for (const auto& [model, _] : value.Data)
		//		{
		//			auto mn = glm::vec3{ model * glm::vec4{ aabb.Min, 1.0f } };
		//			auto mx = glm::vec3{ model * glm::vec4{ aabb.Max, 1.0f } };
		//			context.Debug.DebugMeshes.FrameCube(mn, mx);
		//		}
		//	}

		//	material->SetUniforms(shader);
		//	mesh->DrawInstanced();
		//}
	}
}