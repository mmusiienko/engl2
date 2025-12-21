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
			auto Key() const
			{
				return Path;
			}
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
		auto Key() const
		{
			return Path;
		}
	};

	template<>
	struct AssetImporter<Model>::Params
	{
		std::filesystem::path Path;
		bool IsInstanced = false;

		Params(std::filesystem::path path) : Path(std::move(path)) {}
		Params(std::filesystem::path path, bool isInstanced) : Path(std::move(path)), IsInstanced(isInstanced) {}
		auto Key() const
		{
			return Path;
		}
	};

	template<>
	struct AssetImporter<Shader>::Params
	{
		std::filesystem::path Directory;
		Params(std::filesystem::path directory) : Directory(std::move(directory)) {}
		auto Key() const
		{
			return Directory;
		}
	};

	template<>
	struct AssetImporter<ComputeShader>::Params
	{
		std::filesystem::path Directory;
		Params(std::filesystem::path directory) : Directory(std::move(directory)) {}
		auto Key() const
		{
			return Directory;
		}
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
}
