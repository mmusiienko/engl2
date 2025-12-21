#include "../AssetImporter.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>

#include "../../renderer/base/Mesh.h"
#include "../../renderer/base/Material.h"

namespace EnGl
{
	static void ProcessModelNode(const std::string& modelDirName, aiNode* node, const aiScene* scene, std::vector<Model::Submesh>& out, bool isInstanced);
	static Model::Submesh ConvertAIMeshToMesh(const std::string& modelDirName, aiMesh* aMesh, const aiScene* scene, bool isInstanced);
	static scope<Material::Base> GetMaterial(aiMaterial* mat, const std::string& modelDirName, bool isInstanced);
	static std::optional<AssetHandle<Texture2D>> TryGetTexture(aiMaterial* mat, aiTextureType type, const std::string& modelDirName);

	Model AssetImporter<Model>::Import(const Params& params)
	{
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(params.Path.string(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_OptimizeGraph | aiProcess_OptimizeMeshes);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			spdlog::error(importer.GetErrorString());
			throw std::runtime_error("Could not load model " + params.Path.string());
		}

		std::vector<Model::Submesh> meshMaterials;
		ProcessModelNode(params.Path.parent_path().string(), scene->mRootNode, scene, meshMaterials, params.IsInstanced);
		return Model{ std::move(meshMaterials), params.IsInstanced };
	}

	static void ProcessModelNode(
		const std::string& modelDirName,
		aiNode* node,
		const aiScene* scene,
		std::vector<Model::Submesh>& out,
		bool isInstanced
	)
	{
		for (size_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];
			Model::Submesh mesh = ConvertAIMeshToMesh(modelDirName, aMesh, scene, isInstanced);
			out.push_back(std::move(mesh));
		}
		for (size_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessModelNode(modelDirName, node->mChildren[i], scene, out, isInstanced);
		}
	}

	static Model::Submesh ConvertAIMeshToMesh(
		const std::string& modelDirName,
		aiMesh* aMesh,
		const aiScene* scene,
		bool isInstanced
	)
	{
		std::vector<Mesh::Vertex> vertices;

		std::vector<Mesh::Index> indices;

		bool hasNormals = aMesh->HasNormals();
		bool hasTextureCoords = aMesh->HasTextureCoords(0);
		for (size_t i = 0; i < aMesh->mNumVertices; i++)
		{
			auto pos = aMesh->mVertices[i];
			glm::vec3 vpos{ pos.x, pos.y, pos.z };
			glm::vec3 vnorm{};
			glm::vec2 vtc{};
			
			if (hasNormals)
			{
				auto norm = aMesh->mNormals[i];
				vnorm = { norm.x, norm.y, norm.z };
			}

			if (hasTextureCoords)
			{
				auto tex = aMesh->mTextureCoords[0][i];
				vtc = { tex.x, tex.y };
			}

			vertices.emplace_back(vpos, vnorm, vtc);
		}

		for (size_t i = 0; i < aMesh->mNumFaces; i++)
		{
			aiFace face = aMesh->mFaces[i];
			for (size_t j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		f32 shininess;
		scope<Material::Base> material = nullptr;
		if (aMesh->mMaterialIndex >= 0)
		{
			auto mat = scene->mMaterials[aMesh->mMaterialIndex];
			
			material = GetMaterial(mat, modelDirName, isInstanced);
		}

		if (!material)
		{
			material = make_scope<Material::Unlit>(isInstanced);
		}

		auto meshHandle = AssetManager::Put(Mesh{
			Mesh::CreationInfo
			{
				.Vertices = vertices,
				.Indices = indices,
				.HasNormals = hasNormals,
				.HasTextureCoords = hasTextureCoords
			}
		});

		auto materialHandle = AssetManager::Put<scope<Material::Base>>(std::move(material));

		return { .Mesh = meshHandle, .Material = materialHandle };
	}

	static scope<Material::Base> GetMaterial(
		aiMaterial* mat,
		const std::string& modelDirName,
		bool isInstanced
	)
	{
		std::optional<AssetHandle<Texture2D>> baseColorOpt = TryGetTexture(mat, aiTextureType_BASE_COLOR, modelDirName);
		if (baseColorOpt.has_value())
		{
			return make_scope<Material::UnlitTextured>(baseColorOpt.value(), isInstanced);
		}

		std::optional<AssetHandle<Texture2D>> diffuseOpt = TryGetTexture(mat, aiTextureType_DIFFUSE, modelDirName);
		std::optional<AssetHandle<Texture2D>> specularOpt = TryGetTexture(mat, aiTextureType_SPECULAR, modelDirName);
		f32 shininess;
		mat->Get(AI_MATKEY_SHININESS, shininess);

		if (diffuseOpt.has_value() && specularOpt.has_value())
		{
			return make_scope<Material::Phong>(diffuseOpt.value(), specularOpt.value(), shininess, isInstanced);
		}
		
		return nullptr;
	}

	static std::optional<AssetHandle<Texture2D>> TryGetTexture(aiMaterial* mat, aiTextureType type, const std::string& modelDirName)
	{
		std::optional < AssetHandle<Texture2D> > textureHandleOpt;

		aiString str;
		mat->GetTexture(type, 0, &str);

		if (str.length > 0)
		{
			auto texPath = std::filesystem::path(modelDirName) / str.C_Str();
			textureHandleOpt = AssetManager::Load<Texture2D>(texPath);
		}

		return textureHandleOpt;
	}
}