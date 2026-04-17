#include "../AssetImporter.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>

#include "../../renderer/base/Mesh.h"
#include "../../renderer/base/Material.h"

namespace EnGl
{
	static void ProcessModelNode(const std::string& modelDirName, aiNode* node, const aiScene* scene, std::vector<Model::Submesh>& out, bool isInstanced, bool flipTextures);
	static Model::Submesh ConvertAIMeshToMesh(const std::string& modelDirName, aiMesh* aMesh, const aiScene* scene, bool isInstanced, bool flipTextures);
	static scope<Material::Base> GetMaterial(aiMaterial* mat, const std::string& modelDirName, bool isInstanced, bool flipTextures);
	static std::optional<AssetHandle<Texture2D>> TryGetTexture(aiMaterial* mat, aiTextureType type, const std::string& modelDirName, bool flipTextures, bool isColor = false);

	static glm::vec3 ToVec3(const aiVector3D& vec3)
	{
		return { vec3.x, vec3.y, vec3.z };
	}

	static Mesh::AABB ToAABB(const aiAABB& aabb)
	{
		return { .Min = ToVec3(aabb.mMin), .Max = ToVec3(aabb.mMax) };
	}

	Model AssetImporter<Model>::Import(const Params& params)
	{
		spdlog::info("Loading model at {}", params.Path.string());
		

		static u32 flags = 
			aiProcess_Triangulate |
			aiProcess_FlipUVs |
			aiProcess_OptimizeGraph | 
			aiProcess_OptimizeMeshes |
			aiProcess_GenBoundingBoxes |
			aiProcess_PreTransformVertices | 
			aiProcess_CalcTangentSpace |
			aiProcess_JoinIdenticalVertices;
		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(params.Path.string(), flags);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			spdlog::error(importer.GetErrorString());
			throw std::runtime_error("Could not load model " + params.Path.string());
		}

		std::vector<Model::Submesh> meshMaterials;
		ProcessModelNode(params.Path.parent_path().string(), scene->mRootNode, scene, meshMaterials, params.IsInstanced, params.FlipTextures);
		return Model{ std::move(meshMaterials), params.IsInstanced };
	}

	static void ProcessModelNode(
		const std::string& modelDirName,
		aiNode* node,
		const aiScene* scene,
		std::vector<Model::Submesh>& out,
		bool isInstanced,
		bool flipTextures
	)
	{

		for (size_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];
			Model::Submesh mesh = ConvertAIMeshToMesh(modelDirName, aMesh, scene, isInstanced, flipTextures);
			out.push_back(std::move(mesh));
		}
		for (size_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessModelNode(modelDirName, node->mChildren[i], scene, out, isInstanced, flipTextures);
		}
	}

	static Model::Submesh ConvertAIMeshToMesh(
		const std::string& modelDirName,
		aiMesh* aMesh,
		const aiScene* scene,
		bool isInstanced,
		bool flipTextures
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
			glm::vec3 tangent{};
			glm::vec3 bitangent{};
			
			if (hasNormals)
			{
				vnorm = ToVec3(aMesh->mNormals[i]);
				tangent = ToVec3(aMesh->mTangents[i]);
				bitangent = ToVec3(aMesh->mBitangents[i]);
			}

			if (hasTextureCoords)
			{
				auto tex = aMesh->mTextureCoords[0][i];
				vtc = { tex.x, tex.y };
			}

			vertices.emplace_back(vpos, vnorm, vtc, tangent, bitangent);
		}

		for (size_t i = 0; i < aMesh->mNumFaces; i++)
		{
			aiFace face = aMesh->mFaces[i];
			for (size_t j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		scope<Material::Base> material = nullptr;
		if (aMesh->mMaterialIndex >= 0)
		{
			auto mat = scene->mMaterials[aMesh->mMaterialIndex];
			material = GetMaterial(mat, modelDirName, isInstanced, flipTextures);
		}

		if (!material)
		{
			material = make_scope<Material::Lit>(isInstanced);
		}

		auto meshHandle = AssetManager::Put(Mesh{
			Mesh::CreationInfo
			{
				.Vertices = vertices,
				.Indices = indices,
				.HasNormals = hasNormals,
				.HasTextureCoords = hasTextureCoords,
				.HasTangents = hasNormals,
				.Aabb = ToAABB(aMesh->mAABB)
			}
		});
		auto materialHandle = AssetManager::PutScope<Material::Base>(std::move(material));

		return { .Mesh = meshHandle, .Material = materialHandle };
	}

	static scope<Material::Base> GetMaterial(
		aiMaterial* mat,
		const std::string& modelDirName,
		bool isInstanced,
		bool flipTextures
	)
	{
		std::optional<AssetHandle<Texture2D>> metalnessOpt = TryGetTexture(mat, aiTextureType_METALNESS, modelDirName, flipTextures);
		std::optional<AssetHandle<Texture2D>> roughnessOpt = TryGetTexture(mat, aiTextureType_DIFFUSE_ROUGHNESS, modelDirName, flipTextures);
		std::optional<AssetHandle<Texture2D>> aoOpt = TryGetTexture(mat, aiTextureType_AMBIENT_OCCLUSION, modelDirName, flipTextures);
		std::optional<AssetHandle<Texture2D>> baseColorOpt = TryGetTexture(mat, aiTextureType_BASE_COLOR, modelDirName, flipTextures, true);
		std::optional<AssetHandle<Texture2D>> normalsOpt = TryGetTexture(mat, aiTextureType_NORMALS, modelDirName, flipTextures);

		if (baseColorOpt.has_value() && metalnessOpt.has_value() && roughnessOpt.has_value())
		{
			auto roughnessHandle = roughnessOpt.value();
			auto metallicHandle = metalnessOpt.value();
			if (roughnessHandle.Id == metallicHandle.Id)
			{
				auto mat = make_scope<Material::PBRTexturedARM>();
				mat->AlbedoHandle = baseColorOpt.value();
				mat->ARMHandle = roughnessHandle;
				if (normalsOpt.has_value())
					mat->NormalsHandle = normalsOpt.value();
				
				return mat;
			}
			else
			{
				auto mat = make_scope<Material::PBRTextured>();
				mat->AlbedoHandle = baseColorOpt.value();
				mat->RoughnessHandle = roughnessOpt.value();
				mat->MetallicHandle = metalnessOpt.value();
				if (aoOpt.has_value())
					mat->AOHandle = aoOpt.value();
				if (normalsOpt.has_value())
					mat->NormalsHandle = normalsOpt.value();
				return mat;
			}
		}
		else if (baseColorOpt.has_value())
		{
			auto mat = make_scope<Material::PBRTexturedARM>();
			mat->AlbedoHandle = baseColorOpt.value();
			mat->ARMHandle = Material::PlaceholderTextures::ARM;
			if (normalsOpt.has_value())
				mat->NormalsHandle = normalsOpt.value();
			return mat;
		}

		std::optional<AssetHandle<Texture2D>> diffuseOpt = TryGetTexture(mat, aiTextureType_DIFFUSE, modelDirName, flipTextures, true);
		std::optional<AssetHandle<Texture2D>> specularOpt = TryGetTexture(mat, aiTextureType_SPECULAR, modelDirName, flipTextures, true);
		f32 shininess;
		mat->Get(AI_MATKEY_SHININESS, shininess);

		if (diffuseOpt.has_value() && specularOpt.has_value())
		{
			return make_scope<Material::Phong>(diffuseOpt.value(), specularOpt.value(), shininess, isInstanced);
		}

		if (diffuseOpt.has_value())
		{
			return make_scope<Material::LitTextured>(diffuseOpt.value(), isInstanced);
		}

		return nullptr;
	}

	static std::optional<AssetHandle<Texture2D>> TryGetTexture(
		aiMaterial* mat,
		aiTextureType type,
		const std::string& modelDirName,
		bool flipTextures,
		bool isColor
	)
	{
		std::optional < AssetHandle<Texture2D> > textureHandleOpt;

		aiString str;
		mat->GetTexture(type, 0, &str);

		if (str.length > 0)
		{
			auto path = std::filesystem::path(str.C_Str());
			auto filename = std::filesystem::path(str.C_Str()).filename();

			std::vector<std::filesystem::path> toTry
			{ 
				path,
				std::filesystem::path(modelDirName).parent_path() / "textures" / filename,
				std::filesystem::path(modelDirName) / "textures" / filename,
				std::filesystem::path(modelDirName) / filename,
			};

			for (const auto& texPath : toTry)
			{
				try
				{
					AssetImporter<Texture2D>::Params params{ texPath, {}, flipTextures, isColor };
					textureHandleOpt = AssetManager::Load<Texture2D>(params);
					return textureHandleOpt;
				}
				catch (const std::exception&)
				{
				}
			}
		}

		return textureHandleOpt;
	}
}