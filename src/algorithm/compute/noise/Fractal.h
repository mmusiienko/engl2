#pragma once
#include "../renderer/base/Texture.h"
#include "../resources/importers/AssetManager.h"
#include "./Voronoi.h"
#include "./Perlin.h"


namespace EnGl
{
	struct FractalNoiseParams
	{
		u32 NOctaves = 8;
		f32 Persistence = 0.5f;
		f32 Lacunarity = 2.0f;

		u32 ChannelFrom = 1;
		u32 ChannelTo = 1;
	};

	class FractalNoise2D
	{
	public:
		static void Fill(AssetHandle<Texture2D> textureA, AssetHandle<Texture2D> baseNoise, const FractalNoiseParams& fParams);
	};

	class FractalNoise3D
	{
	public:
		static void Fill(AssetHandle<Texture3D> textureA, AssetHandle<Texture3D> baseNoise, const FractalNoiseParams& fParams);
	};

	struct VoronoiWrapper
	{
		VoronoiNoiseParams ParamsR{ .NPoints = 4, .NChannel = 1 };
		FractalNoiseParams FParamsR{ .ChannelFrom = 1, .ChannelTo = 1 };
		VoronoiNoiseParams ParamsG{ .NPoints = 4, .NChannel = 2 };
		FractalNoiseParams FParamsG{ .ChannelFrom = 2, .ChannelTo = 2 };
		VoronoiNoiseParams ParamsB{ .NPoints = 6, .NChannel = 3 };
		FractalNoiseParams FParamsB{ .ChannelFrom = 3, .ChannelTo = 3 };
		VoronoiNoiseParams ParamsA{ .NPoints = 8, .NChannel = 4 };
		FractalNoiseParams FParamsA{ .ChannelFrom = 4, .ChannelTo = 4 };

		u32 Dim = 128;

		AssetHandle<Texture3D> Voronoi = 
			AssetManager::Put<Texture3D>(
				Dim, 
				Texture::CreationInfoFromData{.CpuFormat = GL_RGBA, .GpuFormat = GL_RGBA32F}
			);
		AssetHandle<Texture3D> VoronoiFBM =
			AssetManager::Put<Texture3D>(
				Dim,
				Texture::CreationInfoFromData{ .CpuFormat = GL_RGBA, .GpuFormat = GL_RGBA32F }
			);

		VoronoiWrapper() 
		{
			VoronoiNoise3D::Fill(Voronoi, ParamsR);
			VoronoiNoise3D::Fill(Voronoi, ParamsG);
			VoronoiNoise3D::Fill(Voronoi, ParamsB);
			VoronoiNoise3D::Fill(Voronoi, ParamsA);

			FractalNoise3D::Fill(VoronoiFBM, Voronoi, FParamsR);
			FractalNoise3D::Fill(VoronoiFBM, Voronoi, FParamsG);
			FractalNoise3D::Fill(VoronoiFBM, Voronoi, FParamsB);
			FractalNoise3D::Fill(VoronoiFBM, Voronoi, FParamsA);
		}
	};

	struct PerlinWrapper
	{
		PerlinNoiseParams ParamsR{ .NCells = 2, .NChannel = 1 };
		FractalNoiseParams FParamsR{ .ChannelFrom = 1, .ChannelTo = 1 };
		PerlinNoiseParams ParamsG{ .NCells = 4, .NChannel = 2 };
		FractalNoiseParams FParamsG{ .ChannelFrom = 2, .ChannelTo = 2 };
		PerlinNoiseParams ParamsB{ .NCells = 6, .NChannel = 3 };
		FractalNoiseParams FParamsB{ .ChannelFrom = 3, .ChannelTo = 3 };
		PerlinNoiseParams ParamsA{ .NCells = 8, .NChannel = 4 };
		FractalNoiseParams FParamsA{ .ChannelFrom = 4, .ChannelTo = 4 };

		u32 Dim = 128;

		AssetHandle<Texture3D> Perlin =
			AssetManager::Put<Texture3D>(
				Dim,
				Texture::CreationInfoFromData{ .CpuFormat = GL_RGBA, .GpuFormat = GL_RGBA32F }
			);
		AssetHandle<Texture3D> PerlinFBM =
			AssetManager::Put<Texture3D>(
				Dim,
				Texture::CreationInfoFromData{ .CpuFormat = GL_RGBA, .GpuFormat = GL_RGBA32F }
			);

		PerlinWrapper()
		{
			PerlinNoise3D::Fill(Perlin, ParamsR);
			PerlinNoise3D::Fill(Perlin, ParamsG);
			PerlinNoise3D::Fill(Perlin, ParamsB);
			PerlinNoise3D::Fill(Perlin, ParamsA);

			FractalNoise3D::Fill(PerlinFBM, Perlin, FParamsR);
			FractalNoise3D::Fill(PerlinFBM, Perlin, FParamsG);
			FractalNoise3D::Fill(PerlinFBM, Perlin, FParamsB);
			FractalNoise3D::Fill(PerlinFBM, Perlin, FParamsA);
		}
	};
}