#pragma once

#include "AssetManager.h"
#include "../renderer/base/Texture.h"
#include "../renderer/base/Model.h"
#include "../renderer/base/ComputeShader.h"
#include "../renderer/base/Cubemap.h"
#include "../FileSystem.h"

namespace EnGl
{
	class AssetManager;

	template<typename AssetT>
	class AssetImporter
	{
	public:
		struct Params
		{
			std::filesystem::path Path;
			Params(std::filesystem::path path) : Path(std::move(path)) {}
			auto operator<=>(const Params&) const = default;
		};

		static AssetT Import(const Params& params);
	};

	template<>
	struct AssetImporter<Texture2D>::Params
	{
		std::filesystem::path Path;
		Texture::CommonInfo TextureParams;

		Params(std::filesystem::path path) : Path(std::move(path)) {}
		Params(std::filesystem::path path, Texture::CommonInfo& params) : Path(std::move(path)), TextureParams(params) {}
		auto operator<=>(const Params&) const = default;
	};

	template<>
	struct AssetImporter<Model>::Params
	{
		std::filesystem::path Path;
		bool IsInstanced = false;

		Params(std::filesystem::path path) : Path(std::move(path)) {}
		Params(std::filesystem::path path, bool isInstanced) : Path(std::move(path)), IsInstanced(isInstanced) {}
		auto operator<=>(const Params&) const = default;
	};

	template<>
	struct AssetImporter<Shader>::Params
	{
		std::filesystem::path Directory;
		Params(std::filesystem::path directory) : Directory(std::move(directory)) {}
		auto operator<=>(const Params&) const = default;
	};

	template<>
	struct AssetImporter<ComputeShader>::Params
	{
		std::filesystem::path Directory;
		Params(std::filesystem::path directory) : Directory(std::move(directory)) {}
		auto operator<=>(const Params&) const = default;
	};

//	template<>
//	class AssetImporter<Cubemap>
//	{
//	public:
//		struct SingleFileParams
//		{
//			const std::filesystem::path& Path;
//
//			auto inline Key() const
//			{
//				return Path;
//			}
//		};
//
//		struct FaceParams
//		{
//			const std::filesystem::path& Path;
//
//			auto inline Key() const
//			{
//				return Path;
//			}
//		};
//
//		Cubemap Import(const FaceParams& params);
//		Cubemap Import(const SingleFileParams& params);
//	};

	template<typename AssetT>
	struct AssetImporterParamsHash
	{
		size_t operator()(const typename AssetImporter<AssetT>::Params& params) const
		{
			size_t res = 0;
			hash_combine(res, params.Path);
			return res;
		}
	};

	template<>
	struct AssetImporterParamsHash<Texture2D>
	{
		size_t operator()(const typename AssetImporter<Texture2D>::Params& params) const
		{
			size_t res = 0;
			hash_combine(res, params.Path);
			hash_combine(res, params.TextureParams);
			return res;
		}
	};

	template<>
	struct AssetImporterParamsHash<Shader>
	{
		size_t operator()(const typename AssetImporter<Shader>::Params& params) const
		{
			size_t res = 0;
			hash_combine(res, params.Directory);
			return res;
		}
	};

	template<>
	struct AssetImporterParamsHash<ComputeShader>
	{
		size_t operator()(const typename AssetImporter<ComputeShader>::Params& params) const
		{
			size_t res = 0;
			hash_combine(res, params.Directory);
			return res;
		}
	};

	template<>
	struct AssetImporterParamsHash<Model>
	{
		size_t operator()(const typename AssetImporter<Model>::Params& params) const
		{
			size_t res = 0;
			hash_combine(res, params.Path);
			hash_combine(res, params.IsInstanced);
			return res;
		}
	};
}


