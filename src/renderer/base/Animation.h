#pragma once
#include "core/Core.h"
#include "resources/importers/AssetManager.h"
#include "renderer/base/Skeleton.h"


namespace EnGl
{
	class Animation
	{
	public:
		template<typename T>
		struct Key
		{
			T Data{};
			f32 Time = 0.0f;
		};

		struct BoneData
		{
			std::vector<Key<glm::vec3>> Positions;
			std::vector<Key<glm::quat>> Rotations;
			std::vector<Key<glm::vec3>> Scales;
		};
	private:
		f32 m_DurationSeconds = 0.0f;
		std::vector<BoneData> m_KeyFrames;
		std::vector<std::string> m_BoneNames;
	public:
		Animation(
			std::vector<BoneData> bones, 
			std::vector<std::string> boneNames,
			f32 durationSeconds
		) : m_KeyFrames(std::move(bones)), m_BoneNames(std::move(boneNames)), m_DurationSeconds(durationSeconds) { }

		const std::vector<BoneData>& KeyFrames() const { return m_KeyFrames; }
		const std::vector<std::string>& BoneNames() const { return m_BoneNames; }
		inline f32 DurationSeconds() const { return m_DurationSeconds; }
	};

	struct AnimationInstance
	{
		AssetHandle<Animation> AnimationHandle;
		std::vector<const Animation::BoneData*> BoneDatas;
		f32 DurationSeconds = 0.0f;

		AnimationInstance(const Animation& animation, const Skeleton& skeleton, AssetHandle<Animation> anim) : DurationSeconds(animation.DurationSeconds()), AnimationHandle(anim)
		{
			const auto& map = skeleton.NameToId();
			const auto& animationBoneNames = animation.BoneNames();
			const auto& keyFrames = animation.KeyFrames();
			BoneDatas.resize(map.size());

			u32 i = 0u;
			for (const auto& name : animationBoneNames)
			{
				if (map.contains(name))
					BoneDatas[map.at(name)] = &keyFrames[i];

				i++;
			}
		}

	private:
		template<typename T>
		using KeyRange = std::pair<Animation::Key<T>, Animation::Key<T>>;

		template<typename T>
		KeyRange<T> FindStartEnd(const std::vector<Animation::Key<T>>& ts, f32 t) const
		{
			KeyRange<T> res;
			size_t idx = std::lower_bound(ts.begin(), ts.end(), t,
				[](const Animation::Key<T>& k, f32 value)
				{
					return value > k.Time;
				}
			) - ts.begin();

			size_t secondIdx = std::min(idx, ts.size() - 1);
			size_t firstIdx = secondIdx > 0 ? secondIdx - 1 : 0;

			res.first = ts[firstIdx];
			res.second = ts[secondIdx];

			return res;
		}

		KeyRange<glm::vec3> FindStartEndPos(u32 bone, f32 t) const { return FindStartEnd(BoneDatas[bone]->Positions, t); }

		KeyRange<glm::quat> FindStartEndRot(u32 bone, f32 t) const { return FindStartEnd(BoneDatas[bone]->Rotations, t); }

		KeyRange<glm::vec3> FindStartEndScale(u32 bone, f32 t) const { return FindStartEnd(BoneDatas[bone]->Scales, t); }

		static f32 NormalizedTime(f32 start, f32 end, f32 t) { return end == start ? 0.0f : (t - start) / (end - start); }
	public:
		glm::vec3 LerpPosition(u32 bone, f32 t, glm::vec3 fallback)
		{
			if (!BoneDatas[bone] || BoneDatas[bone]->Positions.empty()) return std::move(fallback);

			auto [first, second] = FindStartEndPos(bone, t);
			return glm::mix(first.Data, second.Data, NormalizedTime(first.Time, second.Time, t));
		};

		glm::quat LerpRotation(u32 bone, f32 t, glm::quat fallback)
		{
			if (!BoneDatas[bone] || BoneDatas[bone]->Rotations.empty()) return std::move(fallback);

			auto [first, second] = FindStartEndRot(bone, t);
			return glm::slerp(first.Data, second.Data, NormalizedTime(first.Time, second.Time, t));
		};

		glm::vec3 LerpScale(u32 bone, f32 t, glm::vec3 fallback)
		{
			if (!BoneDatas[bone] || BoneDatas[bone]->Scales.empty()) return std::move(fallback);

			auto [first, second] = FindStartEndScale(bone, t);
			return glm::mix(first.Data, second.Data, NormalizedTime(first.Time, second.Time, t));
		};
	};
}