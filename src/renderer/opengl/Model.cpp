#include "../base/Model.h"
#include "../base/Material.h"
#include "../base/Shader.h"


namespace EnGl
{
	Model::Model(std::vector<Submesh>&& meshes, bool isInstanced) noexcept : m_Meshes(std::move(meshes)), m_IsInstanced(isInstanced)
	{

	}

	Model::Model(Submesh&& mesh, bool isInstanced) noexcept : Model(std::vector<Submesh> {std::move(mesh)}, isInstanced)
	{
	}

	void Model::AddMesh(Submesh&& mesh) noexcept
	{
		m_Meshes.push_back(std::move(mesh));
	}

	void Model::SetMeshes(std::vector<Submesh>&& meshes) noexcept
	{
		m_Meshes = std::move(meshes);
	}
}