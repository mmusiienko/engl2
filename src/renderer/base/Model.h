#pragma once

#include "resources/importers/AssetHandle.h"


namespace EnGl
{
	class Mesh;
	class Shader;
	namespace Material { struct Base; }

	class Model
	{
	public:
		struct Submesh
		{
			AssetHandle<Mesh> Mesh;
			AssetHandle<Material::Base> Material;
		};

		Model(std::vector<Submesh>&& meshes, bool IsInstanced = false) noexcept;
		Model(Submesh&& mesh, bool IsInstanced = false) noexcept;
		void AddMesh(Submesh&& mesh) noexcept;
		void SetMeshes(std::vector<Submesh>&& meshes) noexcept;
		Submesh& GetSubmesh(u32 idx) { return m_Meshes[idx]; }

		inline u32 TotalMeshes() const { return static_cast<u32>(m_Meshes.size()); }
		inline bool IsInstanced() const { return m_IsInstanced; }
	private:
		bool m_IsInstanced = false;
		std::vector<Submesh> m_Meshes;
	};
}