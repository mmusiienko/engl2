#include "../base/Model.h"
#include "../base/Material.h"
#include "../base/Shader.h"


namespace EnGl
{
	Model::Model(std::vector<Submesh> meshes, std::vector<AssetHandle<Skeleton>> skeletons, std::vector<AssetHandle<Animation>> animations, bool isInstanced) :
		m_Meshes(std::move(meshes)),
		m_Animations(std::move(animations)),
		m_Skeletons(std::move(skeletons)),
		m_IsInstanced(isInstanced)
	{

	}

	Model::Model(Submesh mesh, bool isInstanced)  : Model(std::vector<Submesh> {std::move(mesh)}, {}, {}, isInstanced)
	{
	}

	void Model::AddMesh(Submesh mesh) 
	{
		m_Meshes.push_back(std::move(mesh));
	}

	void Model::SetMeshes(std::vector<Submesh> meshes) 
	{
		m_Meshes = std::move(meshes);
	}
}