#pragma once
#include "../../resources/importers/AssetImporter.h"
#include "../../math/Random.h"


namespace EnGl
{
	namespace Noise
	{
		struct Gaussian
		{
			static AssetHandle<Texture2D> Get(u32 N)
			{
				static std::unordered_map<u32, AssetHandle<Texture2D>> map;

				if (!map.contains(N))
				{
					std::vector<glm::vec2> data(N * N);
					GaussianRandom rand;

					for (size_t i = 0; i < data.size(); i++)
					{
						data[i] = { rand.GaussianPair() };
						rand.AddToState(static_cast<u32>(i));
					}

					auto handle = AssetManager::Put<Texture2D>(
						N, N, Texture::CreationInfoFromData
						{
							.Data = data.data(),
							.CpuFormat = GL_RG,
							.GpuFormat = GL_RG32F
						}
					);

					map.insert({ N, handle });
				}

				return map.at(N);
			}
		};
	}
}
