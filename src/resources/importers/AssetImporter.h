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
		bool Flip = false;
		bool IsColor = false;
		Texture::CommonInfo TextureParams;
		Params(std::filesystem::path path, Texture::CommonInfo params = {}, bool flip = false, bool isColor = false) 
			: Path(std::move(path)), TextureParams(std::move(params)), Flip(flip), IsColor(isColor) {}
		auto operator<=>(const Params&) const = default;
	};

	template<>
	struct AssetImporter<Model>::Params
	{
		std::filesystem::path Path;
		bool IsInstanced = false;
		bool FlipTextures = true;
		Params(std::filesystem::path path, bool isInstanced = false, bool flipTextures = false) 
			: Path(std::move(path)), IsInstanced(isInstanced), FlipTextures(flipTextures) {}
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

	template<>
	struct AssetImporter<Cubemap>::Params
	{
		std::filesystem::path Path;
		std::vector<std::filesystem::path> Faces;
		auto operator<=>(const Params&) const = default;
		Params(std::filesystem::path path) : Path(std::move(path)) {}
		Params(std::vector<std::filesystem::path> faces) : Faces(std::move(faces)) { assert(Faces.size() == 6); }

	private:
		bool SingleFile = true;
	};

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
			hash_combine(res, params.Flip);
			hash_combine(res, params.IsColor);
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
			hash_combine(res, params.FlipTextures);
			return res;
		}
	};

	template<>
	struct AssetImporterParamsHash<Cubemap>
	{
		size_t operator()(const typename AssetImporter<Cubemap>::Params& params) const
		{
			size_t res = 0;
			hash_combine(res, params.Path);
			for (const auto& face : params.Faces)
			{
				hash_combine(res, face);
			}
			return res;
		}
	};
}


