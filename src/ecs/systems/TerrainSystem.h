#pragma once

#include <vector>

#include "ecs/systems/Systems.h"
#include "algorithm/compute/noise/NoiseTexture.h"


namespace EnGl
{
	class TerrainSystem : public SystemImpl
	{
	public:
		void Run(EcsImpl::EntityManager& manager, GameContext& context) override;
		void Init(EcsImpl::EntityManager& manager) override;
		void Editor(EcsImpl::EntityManager& manager, GameContext& context) override;

		Entity m_Surface = 0;

		struct Props
		{
			Noise2DWrapper TerrainInfo
			{
				.Channels =
				{
					Noise2DChannel{.Params = {.NCells = 32u, .NChannel = 0u, .Seed = 0u, .Octaves = 8u }, .Strategy = Noise2D::Perlin},
					Noise2DChannel{.Params = {.NCells = 4u, .NChannel = 1u, .Seed = 123u, .Octaves = 8u}, .Strategy = Noise2D::Perlin},
					Noise2DChannel{.Params = {.NCells = 16u, .NChannel = 2u, .Seed = 187u, .Octaves = 8u}, .Strategy = Noise2D::Worley},
					Noise2DChannel{.Params = {.NCells = 16u, .NChannel = 3u, .Seed = 13u, .Octaves = 8u}, .Strategy = Noise2D::Worley}
				},
				.Texture = AssetManager::Put<Texture2D>(512u, Texture::CreationInfoFromData{.CpuFormat = GL_RGBA, .GpuFormat = GL_RGBA32F })
			};
			u32 TerrainResolution = 200u;

			glm::vec4 Scale { 0.003f, 0.1f, 0.1f, 1.0f };
			glm::vec4 HeightWeights = { 7000.0f, 3000.0f, 700.0f, 30.0f };
			f32 Resolution = 1.0f;
		};

		Props m_Props{};
		friend struct TerrainSurface;
	};
}