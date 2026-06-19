#include "Animation.h"
#include "renderer/base/Animation.h"
#include "renderer/base/Skeleton.h"
#include "renderer/base/Model.h"


namespace EnGl::System
{
	struct AnimationData
	{
		Mesh* Mesh;
		AnimationInstance* AnimInstance;
		SkeletonInstance* SkeletonInstance;
		Skeleton* Skeleton;
	};

	struct FuncAnimationData
	{
		AnimationInstance* AnimInstance;
		SkeletonInstance* SkeletonInstance;
		Skeleton* Skeleton;
		Entity BoneEntity;
		u32 BoneId;
	};

	static void HandleAnimationSubmesh(
		AnimationData animData,
		std::function<bool(AnimationData&)> before,
		std::function<void(FuncAnimationData)> forEach,
		std::function<void(AnimationData&) > after
	)
	{
		u32 i = 0u;

		if (!before(animData)) return;

		for (auto boneEntity : animData.SkeletonInstance->BoneEntities)
		{
			forEach(FuncAnimationData{.AnimInstance = animData.AnimInstance, .SkeletonInstance = animData.SkeletonInstance, .Skeleton = animData.Skeleton, .BoneEntity = boneEntity, .BoneId = i});
			i++;
		}

		after(animData);
	}

	static void HandleAnimation(
		Component::RenderedModel& rm,
		Component::AnimationData& ad,
		std::function<bool(AnimationData&)> before,
		std::function<void(FuncAnimationData)> forEach,
		std::function<void(AnimationData&) > after
	)
	{
		auto model = AssetManager::GetAsset(rm.Model).Asset;
		auto skelI = AssetManager::GetAsset(ad.Skeleton).Asset;
		auto anim = AssetManager::GetAsset(ad.Animation).Asset;
		if (!model || !skelI || !anim) return;
		auto skel = AssetManager::GetAsset(skelI->SkeletonHandle).Asset;
		if (!skel) return;

		if (rm.MeshIdx == -1)
		{
			for (u32 i = 0; i < model->TotalMeshes(); i++)
			{
				auto mesh = AssetManager::GetAsset(model->GetSubmesh(i).Mesh).Asset;
				if (!mesh) continue;

				HandleAnimationSubmesh(AnimationData{ .Mesh = mesh, .AnimInstance = anim, .SkeletonInstance = skelI, .Skeleton = skel }, before, forEach, after);
			}
		}
		else
		{
			auto mesh = AssetManager::GetAsset(model->GetSubmesh(rm.MeshIdx).Mesh).Asset;
			if (!mesh) return;

			HandleAnimationSubmesh(AnimationData{ .Mesh = mesh, .AnimInstance = anim, .SkeletonInstance = skelI, .Skeleton = skel }, before, forEach, after);
		}
	}

	void PlayAnimation::Run(EcsImpl::EntityManager& manager, GameContext& context)
	{
		auto q = manager.Query<Component::RenderedModel, Component::AnimationData>();
		for (auto [e, rm, a] : q)
		{
			HandleAnimation(rm, a,
				[&](AnimationData& data)
				{
					if (a.Elapsed >= data.AnimInstance->DurationSeconds)
						return false;

					switch (a.Behavior)
					{
					case Component::AnimationData::AnimationBehavior::CLAMP:
						a.Elapsed = glm::min(a.Elapsed + context.DeltaTime * a.Speed, data.AnimInstance->DurationSeconds);
						break;
					case Component::AnimationData::AnimationBehavior::LOOP:
						a.Elapsed = glm::mod(a.Elapsed + context.DeltaTime * a.Speed, data.AnimInstance->DurationSeconds);
						break;
					default:
						break;
					}

					return true;
				},
				[&](FuncAnimationData data)
				{
					auto& transform = manager.Get<Component::Transform>(data.BoneEntity);

					transform.Dirty = true;

					transform.Position = data.AnimInstance->LerpPosition(data.BoneId, a.Elapsed, std::move(transform.Position));
					transform.Rotation = data.AnimInstance->LerpRotation(data.BoneId, a.Elapsed, std::move(transform.Rotation));
					transform.Scale = data.AnimInstance->LerpScale(data.BoneId, a.Elapsed, std::move(transform.Scale));
				},
				[](auto&) {}
			);
		}
	}

	void CollectAnimationTransforms::Run(EcsImpl::EntityManager& manager, GameContext& context)
	{
		auto q = manager.Query<Component::RenderedModel, Component::AnimationData>();
		for (auto [e, rm, a] : q)
		{
			HandleAnimation(rm, a,
				[&](AnimationData& data)
				{ 
					return true;
				},
				[&](FuncAnimationData data)
				{
					auto& model = manager.Get<Component::ModelMatrix>(data.BoneEntity).CachedModel;
					
					data.SkeletonInstance->CurrentBoneTransforms[data.BoneId] = 
						data.Skeleton->GlobalInverseTransform() *
						model *
						data.Skeleton->DefaultBoneTransforms()[data.BoneId];
				},
				[&](AnimationData& data)
				{
					auto ssbo = AssetManager::GetAsset<SSBO>(data.SkeletonInstance->BoneTransforms).Asset;
					if (!ssbo) return;

					ssbo->Resize((void*)data.SkeletonInstance->CurrentBoneTransforms.data(), data.SkeletonInstance->CurrentBoneTransforms.size() * sizeof(glm::mat4));
				}
			);
		}
	}
}

namespace EnGl
{
	static void AddBoneEntities(
		EcsImpl::EntityManager& manager,
		Entity parent,
		Skeleton::Hierarchy::Bone bone,
		const std::unordered_map<std::string, u32>& nameToId,
		std::vector<u32>& out
	)
	{
		Entity entity = 0u;

		if (!bone.Children.empty())
		{
			entity = manager.Create<Component::Children, Component::Transform, Component::LocalModelMatrix, Component::ModelMatrix>(
				[&](Component::Children& children, Component::Transform& transform, auto&...)
				{ 
					children.Children.reserve(bone.Children.size());
					transform.Position = bone.Position;
					transform.Rotation = bone.Rotation;
					transform.Scale = bone.Scale;
					transform.Dirty = true;
				}
			, bone.Name);
		}
		else
		{
			entity = manager.Create<Component::Transform, Component::LocalModelMatrix, Component::ModelMatrix>(
				[&](Component::Transform& transform, auto&...)
				{
					transform.Position = bone.Position;
					transform.Rotation = bone.Rotation;
					transform.Scale = bone.Scale;
					transform.Dirty = true;
				}
			, bone.Name);
		}

		if (nameToId.contains(bone.Name))
		{
			out[nameToId.at(bone.Name)] = entity;
		}

		for (const auto& child : bone.Children)
		{
			AddBoneEntities(manager, entity, child, nameToId, out);
		}

		manager.Get<Component::Children>(parent).Children.push_back(entity);
	}
}

void EnGl::Component::AddAnimationData(EcsImpl::EntityManager& manager, Entity e, AssetHandle<Animation> animH, AssetHandle<Skeleton> skelH)
{
	Animation* anim = AssetManager::GetAsset(animH).Asset;
	Skeleton* skel = AssetManager::GetAsset(skelH).Asset;
	if (!anim || !skel) return;
	
	SkeletonInstance skelInstance;
	skelInstance.BoneEntities.resize(skel->DefaultBoneTransforms().size());
	skelInstance.CurrentBoneTransforms.resize(skel->DefaultBoneTransforms().size(), glm::mat4{ 1.0f });
	skelInstance.SkeletonHandle = skelH;
	skelInstance.BoneTransforms = AssetManager::Put<SSBO>(nullptr, 0u);

	if (!skel->GetHierarchy().Root) return;

	if (!manager.Has<Component::Children>(e))
	{
		manager.Add<Component::Children>(e);
	}

	AddBoneEntities(manager, e, *skel->GetHierarchy().Root, skel->NameToId(), skelInstance.BoneEntities);

	AssetHandle<AnimationInstance> animInstanceH = AssetManager::Put<AnimationInstance>(*anim, *skel, animH);
	AssetHandle<SkeletonInstance> skelInstanceH = AssetManager::Put<SkeletonInstance>(std::move(skelInstance));

	manager.Add<Component::AnimationData>(e, [&](Component::AnimationData& data)
	{
		data.Animation = animInstanceH;
		data.Skeleton = skelInstanceH;
	});

	spdlog::info(manager.Has<Component::AnimationData> (e));
}
