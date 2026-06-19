#pragma once

#include "resources/importers/AssetHandle.h"

namespace EnGl
{
	class Mesh;
	class Shader;
	namespace Material { struct Base; }
	class Animation;
	class Skeleton;

	class Model
	{
	public:
		struct Submesh
		{
			AssetHandle<Mesh> Mesh{};
			AssetHandle<Material::Base> Material{};
		};

		Model(std::vector<Submesh> meshes, std::vector<AssetHandle<Skeleton>> skeletons = {}, std::vector<AssetHandle<Animation>> animations = {}, bool isInstanced = false);
		Model(Submesh mesh, bool IsInstanced = false);
		void AddMesh(Submesh mesh);
		void SetMeshes(std::vector<Submesh> meshes);
		Submesh& GetSubmesh(u32 idx) { return m_Meshes[idx]; }

		inline u32 TotalMeshes() const { return static_cast<u32>(m_Meshes.size()); }
		inline bool IsInstanced() const { return m_IsInstanced; }
	private:
		bool m_IsInstanced = false;
		std::vector<Submesh> m_Meshes;

		std::vector<AssetHandle<Animation>> m_Animations;
		std::vector<AssetHandle<Skeleton>> m_Skeletons;

	public:
		const std::vector<AssetHandle<Animation>>& Animations() const { return m_Animations; }
		const std::vector<AssetHandle<Skeleton>>& Skeletons() const { return m_Skeletons; }
	};
}