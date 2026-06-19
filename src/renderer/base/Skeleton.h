#pragma once
#include "resources/importers/AssetManager.h"


namespace EnGl
{
	class SSBO;

	class Skeleton
	{
		std::unordered_map<std::string, u32> m_NameToId;

		std::vector<glm::mat4> m_DefaultBoneTransforms;
		glm::mat4 m_GlobalInverseTransform;

	public:
		struct Hierarchy
		{
			struct Bone
			{
				std::string Name;
				glm::vec3 Position;
				glm::quat Rotation;
				glm::vec3 Scale;
				std::list<Bone> Children;

				Bone(std::string name) : Name(name) {}
			};

			scope<Bone> Root;
		};
	private:
		Hierarchy m_BoneHierarchy;

	public:
		void Reserve(size_t size)
		{
			m_NameToId.reserve(size);
			m_DefaultBoneTransforms.reserve(size);
		}

		u32 Add(std::string name, glm::mat4 transform)
		{
			u32 id = static_cast<u32>(m_DefaultBoneTransforms.size());
			m_NameToId.insert({ std::move(name), id });
			m_DefaultBoneTransforms.emplace_back(std::move(transform));
			return id;
		}

		void UploadHierarchy(Hierarchy hierarchy)
		{
			m_BoneHierarchy = std::move(hierarchy);
		}

		const std::vector<glm::mat4>& DefaultBoneTransforms() const { return m_DefaultBoneTransforms; }
		std::vector<glm::mat4>& DefaultBoneTransforms() { return m_DefaultBoneTransforms; }
		const std::unordered_map<std::string, u32>& NameToId() const { return m_NameToId; }
		const Hierarchy& GetHierarchy() const { return m_BoneHierarchy; }
		Hierarchy& GetHierarchy() { return m_BoneHierarchy; }

		glm::mat4& GlobalInverseTransform() {return m_GlobalInverseTransform;}
	};

	struct SkeletonInstance
	{
		std::vector<glm::mat4> CurrentBoneTransforms;
		std::vector<u32> BoneEntities;

		AssetHandle<SSBO> BoneTransforms;

		AssetHandle<Skeleton> SkeletonHandle;
	};
}