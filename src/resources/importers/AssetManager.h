#pragma once

#include <queue>
#include <algorithm>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <ranges>

#include "core/Core.h"
#include "resources/importers/AssetImporter.h"
#include "resources/importers/AssetHandle.h"


namespace EnGl
{
	template<typename AssetT>
	class AssetImporter;

	template<typename AssetT>
	struct AssetImporterParamsHash;

	class AssetManager
	{
	private:
		static constexpr std::string_view DEFAULT_ASSET_NAME{ "default" };

		template<typename AssetT>
		struct Storage
		{
			std::unordered_map<
				typename AssetImporter<AssetT>::Params,
				AssetId,
				AssetImporterParamsHash<AssetT>
			> PathToAsset;

			std::unordered_map<
				AssetId,
				typename AssetImporter<AssetT>::Params
			> AssetToPath;

			std::vector<scope<AssetT>> Assets;
			std::vector<u32> Generations{ 0 };
			std::vector<std::string> Names{ DEFAULT_ASSET_NAME.data()};
			std::vector<bool> Alive{ false };
			std::queue<AssetId> DeadIds;

			Storage() { Assets.emplace_back(nullptr); }
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
		static std::vector<AssetHandle<AssetT>> ListAssets()
		{
			std::vector<AssetHandle<AssetT>> res;
			auto& storage = GetAssetStorage<AssetT>();
			u32 i = 1;
			for (auto [alive, gen] : std::views::zip(storage.Alive, storage.Generations) | std::views::drop(1))
			{
				if (alive)
					res.emplace_back(i, gen);
				i++;
			}

			return res;
		}

		template<typename AssetT>
		static const std::string& GetName(AssetHandle<AssetT> handle)
		{
			auto& storage = GetAssetStorage<AssetT>();
			return storage.Names[handle.Id];
		}

		template<typename AssetT>
		static AssetResult<AssetT> GetAsset(AssetHandle<AssetT> handle)
		{
			auto& storage = GetAssetStorage<AssetT>();
			AssetResult<AssetT> res{};

			if (storage.Alive[handle.Id])
			{
				res.Asset = storage.Assets[handle.Id].get();
				res.CurrentGeneration = storage.Generations[handle.Id];
			}

			return res;
		}

		template<typename AssetT>
		static AssetT* GetAssetNoCheck(AssetHandle<AssetT> handle)
		{
			auto& storage = GetAssetStorage<AssetT>();
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
			storage.PathToAsset.insert({ params, handle.Id });
			storage.AssetToPath.insert({ handle.Id, std::move(params) });

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
					storage.Assets.push_back(make_scope<AssetT>(std::move(asset)));
					storage.Generations.push_back(gen);
					storage.Alive.push_back(true);
					storage.Names.push_back(DEFAULT_ASSET_NAME.data());
				}
				else
				{
					id = storage.DeadIds.front();
					storage.DeadIds.pop();
					storage.Assets[id] = make_scope<AssetT>(std::move(asset));
					storage.Alive[id] = true;
					storage.Names[id] = DEFAULT_ASSET_NAME;
					gen = storage.Generations[id]++;
				}

				return AssetHandle<AssetT> {id, gen};
			}
			template<typename AssetT>
			static AssetHandle<AssetT> PutInternal(scope<AssetT>&& asset)
			{
				auto& storage = GetAssetStorage<AssetT>();

				AssetId id = 0;
				u32 gen = 0;
				if (storage.DeadIds.empty())
				{
					id = static_cast<AssetId>(storage.Assets.size());
					storage.Assets.push_back(std::move(asset));
					storage.Generations.push_back(gen);
					storage.Alive.push_back(true);
					storage.Names.push_back(DEFAULT_ASSET_NAME.data());
				}
				else
				{
					id = storage.DeadIds.front();
					storage.DeadIds.pop();
					storage.Assets[id] = std::move(asset);
					storage.Alive[id] = true;
					storage.Names[id] = DEFAULT_ASSET_NAME;
					gen = storage.Generations[id]++;
				}

				return AssetHandle<AssetT> {id, gen};
			}
		public:

		template<typename AssetT>
		static void Remove(AssetHandle<AssetT> asset)
		{
			auto& storage = GetAssetStorage<AssetT>();

			if (!storage.Alive[asset.Id]) return;

			if (storage.AssetToPath.contains(asset.Id))
			{
				auto& params = storage.AssetToPath.at(asset.Id);
				storage.PathToAsset.erase(params);
				storage.AssetToPath.erase(asset.Id);
			}

			storage.Assets[asset.Id] = nullptr;
			storage.Alive[asset.Id] = false;
			storage.DeadIds.push(asset.Id);
		}

		template<typename AssetT>
		static void ReloadAll()
		{
			auto& storage = GetAssetStorage<AssetT>();

			for (const auto& [importParams, id] : storage.PathToAsset)
			{
				storage.Assets[id] = make_scope<AssetT>(AssetImporter<AssetT>::Import(importParams));
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
		static AssetHandle<AssetT> PutScope(scope<AssetT>&& asset)
		{
			return PutInternal<AssetT>(std::move(asset));
		}

		static void ShutDown()
		{
			std::for_each(CleanUps.begin(), CleanUps.end(), [](auto el) { el(); });
		}

		inline static const std::filesystem::path RESOURCES_DIR = std::filesystem::path{ "resources" };

			inline static const std::filesystem::path ASSETS_DIR = RESOURCES_DIR / "assets";
				inline static const std::filesystem::path TEXTURE_DIR = ASSETS_DIR / "textures";
				inline static const std::filesystem::path MODEL_DIR = ASSETS_DIR / "models";

			inline static const std::filesystem::path SHADER_DIR = RESOURCES_DIR / "shaders";
				inline static const std::filesystem::path GRAPHICS_SHADER_DIR = AssetManager::SHADER_DIR / "graphics";
				inline static const std::filesystem::path COMPUTE_SHADER_DIR = AssetManager::SHADER_DIR / "compute";
	};
}