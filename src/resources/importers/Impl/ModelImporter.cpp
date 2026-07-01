#include "resources/importers/AssetImporter.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>

#include "renderer/base/Mesh.h"
#include "renderer/base/Material.h"

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include <glm/gtx/matrix_decompose.hpp>


namespace EnGl
{
	static void ProcessModelNode(
		const std::string& modelDirName,
		aiNode* node,
		const aiScene* scene,
		std::vector<Model::Submesh>& out,
		const AssetImporter<Model>::Params& params,
		std::unordered_map<std::uintptr_t, Skeleton*>& boneToSkeleton,
		Skeleton::Hierarchy::Bone* parent = nullptr,
		Skeleton* skeleton = nullptr,
		glm::mat4 accumulated = glm::mat4{1.0f}
	);
	static Model::Submesh ConvertAIMeshToMesh(
		const std::string& modelDirName,
		aiMesh* aMesh,
		const aiScene* scene,
		const AssetImporter<Model>::Params& params,
		std::unordered_map<std::uintptr_t, Skeleton*>& boneToSkeleton
	);
	static scope<Material::Base> GetMaterial(const aiScene* scene, aiMaterial* mat, const std::string& modelDirName, const AssetImporter<Model>::Params& params);
	static std::optional<AssetHandle<Texture2D>> TryGetTexture(const aiScene* scene, aiMaterial* mat, aiTextureType type, const std::string& modelDirName, bool flip, bool isColor = false);
	static void ProcessAnimations(const aiScene* scene, std::vector<AssetHandle<Animation>>& out);
	static void PrebuildBoneMap(
		const aiScene* scene, const aiNode* node,
		std::vector<AssetHandle<Skeleton>>& skeletons,
		std::unordered_map<std::uintptr_t, Skeleton*>& skeletonPtrs,
		std::unordered_map<std::uintptr_t, Skeleton*>& boneToSkeleton
	);

	static glm::vec3 ToVec3(const aiVector3D& vec3)
	{
		return { vec3.x, vec3.y, vec3.z };
	}

	static glm::quat ToQuat(const aiQuaternion& quat)
	{
		return glm::quat{ quat.w, quat.x, quat.y, quat.z  };
	}

	static glm::mat4 ToMat4(const aiMatrix4x4& mat)
	{
		return glm::mat4(
			mat.a1, mat.b1, mat.c1, mat.d1,
			mat.a2, mat.b2, mat.c2, mat.d2,
			mat.a3, mat.b3, mat.c3, mat.d3,
			mat.a4, mat.b4, mat.c4, mat.d4 
		);
	}

	static Mesh::AABB ToAABB(const aiAABB& aabb)
	{
		return { .Min = ToVec3(aabb.mMin), .Max = ToVec3(aabb.mMax) };
	}

	Model AssetImporter<Model>::Import(const Params& params)
	{
		spdlog::info("Loading model at {}", params.Path.string());
		

		u32 flags = 
			aiProcess_Triangulate |
			aiProcess_FlipUVs |
			aiProcess_GenBoundingBoxes |
			aiProcess_CalcTangentSpace;

		if (!params.ImportAnimations)
		{
			flags |= aiProcess_JoinIdenticalVertices | aiProcess_PreTransformVertices | aiProcess_OptimizeMeshes | aiProcess_OptimizeGraph;
		}
		else
		{
			flags |= aiProcess_PopulateArmatureData;
		}

		Assimp::Importer importer;

		const aiScene* scene = importer.ReadFile(params.Path.string(), flags);
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			spdlog::error(importer.GetErrorString());
			throw std::runtime_error("Could not load model " + params.Path.string());
		}

		std::vector<Model::Submesh> submeshes;
		std::vector<AssetHandle<Skeleton>> skeletons;
		std::vector<AssetHandle<Animation>> animations;
		std::unordered_map<std::uintptr_t, Skeleton*> armatureToSkeleton;
		std::unordered_map<std::uintptr_t, Skeleton*> boneToSkeleton;

		if (params.ImportAnimations)
		{
			PrebuildBoneMap(scene, scene->mRootNode, skeletons, armatureToSkeleton, boneToSkeleton);
		}

		ProcessModelNode(params.Path.parent_path().string(), scene->mRootNode, scene, submeshes, params, boneToSkeleton);

		if (params.ImportAnimations)
		{
			ProcessAnimations(scene, animations);
		}

		return Model{ std::move(submeshes), std::move(skeletons), std::move(animations), params.IsInstanced};
	}

	static void PrebuildBoneMap(
		const aiScene* scene, const aiNode* node,
		std::vector<AssetHandle<Skeleton>>& skeletons,
		std::unordered_map<std::uintptr_t, Skeleton*>& armatureToSkeleton,
		std::unordered_map<std::uintptr_t, Skeleton*>& boneToSkeleton
	)
	{
		for (u32 i = 0u; i < node->mNumMeshes; i++)
		{
			auto& mesh = scene->mMeshes[node->mMeshes[i]];
			Skeleton* skeleton = nullptr;
			if (mesh->mNumBones > 0)
			{
				std::uintptr_t armId = reinterpret_cast<std::uintptr_t>(mesh->mBones[0]->mArmature);
				if (!armatureToSkeleton.contains(armId))
				{
					skeletons.push_back(AssetManager::Put<Skeleton>());
					skeleton = AssetManager::GetAsset(skeletons.back()).Asset;
					if (!skeleton)
					{
						spdlog::error("Error creating skeleton");
						return;
					}

					armatureToSkeleton.insert({ armId, skeleton });

					glm::mat4 globalInverse = glm::inverse(ToMat4(mesh->mBones[0]->mArmature->mTransformation));
					skeleton->GlobalInverseTransform() = std::move(globalInverse);
				}
				else
				{
					skeleton = armatureToSkeleton.at(armId);
				}
			}

			for (u32 j = 0u; j < mesh->mNumBones; j++)
			{
				std::string name = mesh->mBones[j]->mName.C_Str();

				if (!skeleton->NameToId().contains(name))
				{
					skeleton->Add(std::move(name), glm::mat4{ ToMat4(mesh->mBones[j]->mOffsetMatrix) });
				}
				std::uintptr_t boneId = reinterpret_cast<std::uintptr_t>(mesh->mBones[j]->mNode);
				boneToSkeleton.insert({ boneId, skeleton });
			}
		}

		for (size_t i = 0; i < node->mNumChildren; i++)
		{
			PrebuildBoneMap(scene, node->mChildren[i], skeletons, armatureToSkeleton, boneToSkeleton);
		}
	}

	static void ProcessAnimations(
		const aiScene* scene, 
		std::vector<AssetHandle<Animation>>& out
	)
	{
		out.reserve(scene->mNumAnimations);
		for (u32 i = 0u; i < scene->mNumAnimations; i++)
		{
			std::vector<Animation::BoneData> boneInfos;
			std::vector<std::string> boneNames;
			boneInfos.reserve(scene->mAnimations[i]->mNumChannels);
			boneNames.reserve(scene->mAnimations[i]->mNumChannels);
			f32 duration = static_cast<f32>(scene->mAnimations[i]->mDuration);
			f32 ticks = static_cast<f32>(scene->mAnimations[i]->mTicksPerSecond);
			if (ticks == 0.0f) ticks = 25.0f;

			f32 durationSecondsAnim = duration / ticks;
			
			for (u32 j = 0u; j < scene->mAnimations[i]->mNumChannels; j++)
			{
				aiNodeAnim* boneInfo = scene->mAnimations[i]->mChannels[j];

				std::vector<Animation::Key<glm::vec3>> pos;
				std::vector<Animation::Key<glm::quat>> rot;
				std::vector<Animation::Key<glm::vec3>> scale;

				pos.reserve(boneInfo->mNumPositionKeys);
				for (u32 k = 0u; k < boneInfo->mNumPositionKeys; k++)
					pos.emplace_back(ToVec3(boneInfo->mPositionKeys[k].mValue), static_cast<f32>(boneInfo->mPositionKeys[k].mTime) / ticks);

				rot.reserve(boneInfo->mNumRotationKeys);
				for (u32 k = 0u; k < boneInfo->mNumRotationKeys; k++)
					rot.emplace_back(ToQuat(boneInfo->mRotationKeys[k].mValue), static_cast<f32>(boneInfo->mRotationKeys[k].mTime) / ticks);

				scale.reserve(boneInfo->mNumScalingKeys);
				for (u32 k = 0u; k < boneInfo->mNumScalingKeys; k++)
					scale.emplace_back(ToVec3(boneInfo->mScalingKeys[k].mValue), static_cast<f32>(boneInfo->mScalingKeys[k].mTime) / ticks);
				boneInfos.emplace_back(pos, rot, scale);
				boneNames.push_back(boneInfo->mNodeName.C_Str());
			}

			out.emplace_back(AssetManager::Put<Animation>(std::move(boneInfos), std::move(boneNames), durationSecondsAnim));
		}
	}

	static void ProcessModelNode(
		const std::string& modelDirName,
		aiNode* node,
		const aiScene* scene,
		std::vector<Model::Submesh>& out,
		const AssetImporter<Model>::Params& params,
		std::unordered_map<std::uintptr_t, Skeleton*>& boneToSkeleton,
		Skeleton::Hierarchy::Bone* parent,
		Skeleton* skeleton,
		glm::mat4 accumulated
	)
	{
		std::string name = node->mName.C_Str();

		std::uintptr_t nodeId = reinterpret_cast<std::uintptr_t>(node);

		glm::mat4 mat = ToMat4(node->mTransformation);

		if (params.ImportAnimations && !skeleton)
		{
			if (boneToSkeleton.contains(nodeId))
			{
				skeleton = boneToSkeleton.at(nodeId);
				skeleton->GlobalInverseTransform() = glm::inverse(accumulated);
			}
			else
			{
				accumulated = accumulated * mat;
			}
		}

		for (size_t i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* aMesh = scene->mMeshes[node->mMeshes[i]];
			Model::Submesh mesh = ConvertAIMeshToMesh(modelDirName, aMesh, scene, params, boneToSkeleton);
			out.push_back(std::move(mesh));
		}

		if (params.ImportAnimations && skeleton)
		{
			if (boneToSkeleton.contains(nodeId))
			{
				if (!parent)
				{
					skeleton->GetHierarchy().Root = make_scope<Skeleton::Hierarchy::Bone>(std::move(name));
					parent = skeleton->GetHierarchy().Root.get();
				}
				else
				{
					parent->Children.emplace_back(std::move(name));
					parent = &parent->Children.back();
				}
			}
			
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(mat, parent->Scale, parent->Rotation, parent->Position, skew, perspective);
		}

		for (size_t i = 0; i < node->mNumChildren; i++)
		{
			ProcessModelNode(modelDirName, node->mChildren[i], scene, out, params, boneToSkeleton, parent, skeleton, accumulated);
		}
	}

	static Model::Submesh ConvertAIMeshToMesh(
		const std::string& modelDirName,
		aiMesh* aMesh,
		const aiScene* scene,
		const AssetImporter<Model>::Params& params,
		std::unordered_map<std::uintptr_t, Skeleton*>& boneToSkeleton
	)
	{
		std::vector<Mesh::Vertex> vertices;

		std::vector<Mesh::Index> indices;
		
		bool hasNormals = aMesh->HasNormals();
		bool hasTangentsAndBitangents = aMesh->HasTangentsAndBitangents();
		bool hasTextureCoords = aMesh->HasTextureCoords(0);
		bool hasBones = aMesh->HasBones();

		std::vector<glm::mat4> boneOffsets;
		std::vector<u32> boneEntities;
		std::vector<glm::mat4> boneTransforms;

		vertices.reserve(aMesh->mNumVertices);
		indices.reserve(aMesh->mNumFaces);

		boneOffsets.reserve(aMesh->mNumBones);
		boneEntities.reserve(aMesh->mNumBones);
		boneTransforms.reserve(aMesh->mNumBones);

		for (size_t i = 0; i < aMesh->mNumVertices; i++)
		{
			auto pos = aMesh->mVertices[i];
			glm::vec3 vpos{ pos.x, pos.y, pos.z };
			glm::vec3 vnorm{};
			glm::vec2 vtc{};
			glm::vec3 tangent{};
			glm::ivec4 boneIds{-1};
			glm::vec4 boneWeights{};
			
			if (hasNormals)
			{
				vnorm = ToVec3(aMesh->mNormals[i]);
				if (hasTangentsAndBitangents)
				{
					tangent = ToVec3(aMesh->mTangents[i]);
				}
			}

			if (hasTextureCoords)
			{
				auto tex = aMesh->mTextureCoords[0][i];
				vtc = { tex.x, tex.y };
			}

			vertices.emplace_back(vpos, vnorm, vtc, tangent, boneIds, boneWeights);
		}

		if (hasBones && params.ImportAnimations)
		{
			std::uintptr_t boneId = reinterpret_cast<std::uintptr_t>(aMesh->mBones[0]->mNode);

			Skeleton* skeleton = boneToSkeleton.at(boneId);

			for (u32 i = 0; i < aMesh->mNumBones; i++)
			{
				const auto& bone = aMesh->mBones[i];
				std::string name = bone->mName.C_Str();
				u32 boneId = skeleton->NameToId().at(name);

				for (u32 j = 0u; j < bone->mNumWeights; j++)
				{
					u32 vertexId = bone->mWeights[j].mVertexId;
					for (u32 k = 0; k < 4u; k++)
					{
						if (vertices[vertexId].BoneIds[k] != -1) continue;

						vertices[vertexId].BoneIds[k] = boneId;
						vertices[vertexId].BoneWeights[k] = bone->mWeights[j].mWeight;
						break;
					}
				}
			}
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
			material = GetMaterial(scene, mat, modelDirName, params);
		}

		if (!material)
		{
			material = make_scope<Material::Lit>(params.IsInstanced);
		}

		auto meshHandle = AssetManager::Put(Mesh{
			Mesh::CreationInfo
			{
				.Vertices = vertices,
				.Indices = indices,
				.HasNormals = hasNormals,
				.HasTextureCoords = hasTextureCoords,
				.HasTangents = hasNormals,
				.HasBones = hasBones,
				.Aabb = ToAABB(aMesh->mAABB)
			}
		});
		auto materialHandle = AssetManager::PutScope<Material::Base>(std::move(material));

		return { .Mesh = meshHandle, .Material = materialHandle };
	}

	static scope<Material::Base> GetMaterial(
		const aiScene* scene,
		aiMaterial* mat,
		const std::string& modelDirName,
		const AssetImporter<Model>::Params& params
	)
	{
		std::optional<AssetHandle<Texture2D>> metalnessOpt = TryGetTexture(scene, mat, aiTextureType_METALNESS, modelDirName, params.FlipTextures);
		std::optional<AssetHandle<Texture2D>> roughnessOpt = TryGetTexture(scene, mat, aiTextureType_DIFFUSE_ROUGHNESS, modelDirName, params.FlipTextures);
		std::optional<AssetHandle<Texture2D>> aoOpt = TryGetTexture(scene, mat, aiTextureType_AMBIENT_OCCLUSION, modelDirName, params.FlipTextures);
		std::optional<AssetHandle<Texture2D>> baseColorOpt = TryGetTexture(scene, mat, aiTextureType_BASE_COLOR, modelDirName, params.FlipTextures, true);
		std::optional<AssetHandle<Texture2D>> normalsOpt = TryGetTexture(scene, mat, aiTextureType_NORMALS, modelDirName, params.FlipTextures);

		if (baseColorOpt.has_value() && metalnessOpt.has_value() && roughnessOpt.has_value())
		{
			auto roughnessHandle = roughnessOpt.value();
			auto metallicHandle = metalnessOpt.value();
			if (roughnessHandle.Id == metallicHandle.Id)
			{
				auto mat = params.ImportAnimations ? make_scope<Material::PBRTexturedARMAnimated>() : make_scope<Material::PBRTexturedARM>();

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
			auto mat = params.ImportAnimations ? make_scope<Material::PBRTexturedARMAnimated>() : make_scope<Material::PBRTexturedARM>();

			mat->AlbedoHandle = baseColorOpt.value();
			mat->ARMHandle = Material::PlaceholderTextures::ARM;
			if (normalsOpt.has_value())
				mat->NormalsHandle = normalsOpt.value();
			return mat;
		}

		std::optional<AssetHandle<Texture2D>> diffuseOpt = TryGetTexture(scene, mat, aiTextureType_DIFFUSE, modelDirName, params.FlipTextures, true);
		std::optional<AssetHandle<Texture2D>> specularOpt = TryGetTexture(scene, mat, aiTextureType_SPECULAR, modelDirName, params.FlipTextures, true);
		f32 shininess;
		mat->Get(AI_MATKEY_SHININESS, shininess);

		if (diffuseOpt.has_value() && specularOpt.has_value())
		{
			auto mat = params.ImportAnimations ? make_scope<Material::PhongAnimated>() : make_scope<Material::Phong>();

			mat->DiffuseHandle = diffuseOpt.value();
			mat->SpecularHandle = specularOpt.value();
			mat->Shininess = shininess;

			if (normalsOpt.has_value())
				mat->NormalsHandle = normalsOpt.value();
			return mat;
		}

		if (diffuseOpt.has_value())
		{
			auto mat = params.ImportAnimations ? make_scope<Material::PBRTexturedARMAnimated>() : make_scope<Material::PBRTexturedARM>();
			mat->AlbedoHandle = diffuseOpt.value();
			mat->ARMHandle = Material::PlaceholderTextures::ARM;
			if (normalsOpt.has_value())
				mat->NormalsHandle = normalsOpt.value();
			return mat;
		}

		return nullptr;
	}

	static std::optional<AssetHandle<Texture2D>> LoadEmbedded(
		const std::string& name,
		const aiTexture* texture,
		bool flipTextures,
		bool isColor
	)
	{
		try
		{
			AssetImporter<Texture2D>::Params params{ name, Texture::CommonInfo{.MinFilter = GL_LINEAR_MIPMAP_LINEAR, .MagFilter = GL_LINEAR_MIPMAP_LINEAR}, flipTextures, isColor, true, {texture->mWidth, reinterpret_cast<unsigned char*>(texture->pcData)} };

			std::optional<AssetHandle<Texture2D>> textureHandleOpt 
				= AssetManager::Load<Texture2D>(params);
			return textureHandleOpt;
		}
		catch (const std::exception&)
		{
			return {};
		}
	}

	static std::optional<AssetHandle<Texture2D>> TryGetTexture(
		const aiScene* scene,
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
			std::string name = str.C_Str();
			

			if (const aiTexture* tex = scene->GetEmbeddedTexture(name.c_str()))
			{
				return LoadEmbedded(name, tex, flipTextures, isColor);
			}

			auto path = std::filesystem::path(name);
			auto filename = std::filesystem::path(name).filename();

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
					AssetImporter<Texture2D>::Params params{ texPath, Texture::CommonInfo{.MinFilter = GL_LINEAR_MIPMAP_LINEAR, .MagFilter = GL_LINEAR_MIPMAP_LINEAR}, flipTextures, isColor};
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