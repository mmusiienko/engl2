#pragma once

#include <queue>
#include "../../core/Core.h"
#include "AssetImporter.h"
#include <algorithm>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include "AssetHandle.h"


namespace EnGl
{
	template<typename AssetT>
	class AssetImporter;

	template<typename AssetT>
	struct AssetImporterParamsHash;

	class AssetManager
	{
	private:

		template<typename AssetT>
		struct Storage
		{
			std::unordered_map<
				typename AssetImporter<AssetT>::Params,
				AssetId,
				AssetImporterParamsHash<AssetT>
			> PathToAsset;
			std::vector<AssetT> Assets;
			std::vector<u32> Generations;
			std::vector<bool> Alive;
			std::queue<AssetId> DeadIds;
		};

		inline static std::vector<std::function<void()>> CleanUps;
		template<typename AssetT>
		static Storage<AssetT>& GetAssetStorage()
		{
			static Storage<AssetT> storage;
			static bool onRegister = (CleanUps.push_back([]{ storage = {}; }), true);

			return storage;
		}

	public:
		template<typename AssetT>
		struct AssetResult
		{
			AssetT* Asset = nullptr;
			u32 CurrentGeneration = 0;
		};

		template<typename AssetT>
		struct AssetResult<scope<AssetT>>
		{
			AssetT* Asset = nullptr;
			u32 CurrentGeneration = 0;
		};

		template<typename AssetT>
		static AssetResult<AssetT> GetAsset(AssetHandle<AssetT> handle)
		{
			auto& storage = GetAssetStorage<AssetT>();
			AssetResult<AssetT> res;

			if (storage.Alive[handle.Id])
			{
				res.Asset = &storage.Assets[handle.Id];
				res.CurrentGeneration = storage.Generations[handle.Id];
			}

			return res;
		}

		template<typename AssetT>
		static AssetT* GetAssetNoCheck(AssetHandle<AssetT> handle)
		{
			auto& storage = GetAssetStorage<AssetT>();
			return &storage.Assets[handle.Id];
		}

		template<typename AssetT>
		static AssetResult<scope<AssetT>> GetAsset(AssetHandle< scope<AssetT> > handle)
		{
			auto& storage = GetAssetStorage<scope<AssetT>>();
			AssetResult<AssetT> res;

			if (storage.Alive[handle.Id])
			{
				res.Asset = storage.Assets[handle.Id].get();
				res.CurrentGeneration = storage.Generations[handle.Id];
			}

			return res;
		}

		template<typename AssetT>
		static AssetT* GetAssetNoCheck(AssetHandle< scope<AssetT> > handle)
		{
			auto& storage = GetAssetStorage<scope<AssetT>>();
			return storage.Assets[handle.Id].get();
		}

		template<typename AssetT>
		static AssetHandle<AssetT> Load(AssetImporter<AssetT>::Params params)
		{
			auto& storage = GetAssetStorage<AssetT>();

			if (storage.PathToAsset.contains(params))
			{
				AssetId id = storage.PathToAsset.at(params);
				u32 gen = storage.Generations[id];
				return AssetHandle<AssetT>
				{
					.Id = id,
					.Generation = gen
				};
			}

			auto asset = AssetImporter<AssetT>::Import(params);
			auto handle = Put<AssetT>(std::move(asset));
			storage.PathToAsset.insert({ std::move(params), handle.Id});

			return handle;
		}

		private:
			template<typename AssetT>
			static AssetHandle<AssetT> PutInternal(AssetT&& asset)
			{
				auto& storage = GetAssetStorage<AssetT>();

				AssetId id = 0;
				u32 gen = 0;
				if (storage.DeadIds.empty())
				{
					id = static_cast<AssetId>(storage.Assets.size());
					storage.Assets.push_back(std::move(asset));
					storage.Generations.push_back(0);
					storage.Alive.push_back(true);
				}
				else
				{
					id = storage.DeadIds.back();
					storage.DeadIds.pop();
					storage.Assets[id] = std::move(asset);
					storage.Alive[id] = true;
					gen = storage.Generations[id]++;
				}

				return AssetHandle<AssetT>
				{
					.Id = id,
						.Generation = gen
				};
			}
		public:

		template<typename AssetT>
		static void ReloadAll()
		{
			auto& storage = GetAssetStorage<AssetT>();

			for (const auto& [importParams, id] : storage.PathToAsset)
			{
				storage.Assets[id] = AssetImporter<AssetT>::Import(importParams);
			}
		}

		template<typename AssetT, typename... Args>
		static AssetHandle<AssetT> Put(Args&&... args)
		{
			return PutInternal<AssetT>(AssetT{ std::forward<Args>(args)... });
		}

		template<typename AssetT>
		static AssetHandle<AssetT> Put(AssetT&& asset)
		{
			return PutInternal<AssetT>(std::move(asset));
		}

		template<typename AssetT>
		static AssetId PutPersistent(AssetT&& asset)
		{
			return Put<AssetT>(std::move(asset)).Id;
		}

		template<typename AssetT, typename Params>
		static AssetId LoadPersistent(const Params& params)
		{
			return Load<AssetT, Params>(params).Id;
		}

		static void ShutDown()
		{
			std::for_each(CleanUps.begin(), CleanUps.end(), [](auto el) { el(); });
		}

		inline static const std::filesystem::path ASSETS_DIR = std::filesystem::path{"resources"} / "assets";
		inline static const std::filesystem::path TEXTURE_DIR = AssetManager::ASSETS_DIR / "textures";
		inline static const std::filesystem::path MODEL_DIR = AssetManager::ASSETS_DIR / "models";
		inline static const std::filesystem::path SHADER_DIR = AssetManager::ASSETS_DIR / "shaders";
		inline static const std::filesystem::path GRAPHICS_SHADER_DIR = AssetManager::SHADER_DIR / "graphics";
		inline static const std::filesystem::path COMPUTE_SHADER_DIR = AssetManager::SHADER_DIR / "compute";
	};
}