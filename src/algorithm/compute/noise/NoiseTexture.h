#pragma once

#include "algorithm/compute/noise/Noise.h"


namespace EnGl
{
	template<typename N2D>
	static Noise2D const* Noise2DSingleton()
	{
		static N2D noise;
		return &noise;
	}

	struct Noise2DChannel
	{
		NoiseParams Params{};
		std::function<void(AssetHandle<Texture2D>, const NoiseParams&)> Strategy = Noise2D::Perlin;
	};

	constexpr static u32 Noise2DChannels = 4u;

	struct Noise2DWrapper
	{
		Noise2DChannel Channels[Noise2DChannels]{};
		AssetHandle<Texture2D> Texture = AssetManager::Put<Texture2D>(128u, Texture::CreationInfoFromData{ .CpuFormat = GL_RGBA, .GpuFormat = GL_RGBA32F });

		void Fill()
		{
			for (const auto& channel : Channels) channel.Strategy(Texture, channel.Params);
		}
	};

	template<typename N3D>
	static Noise3D const* Noise3DSingleton()
	{
		static N3D noise;
		return &noise;
	}

	struct Noise3DChannel
	{
		NoiseParams Params{};
		std::function<void(AssetHandle<Texture3D>, const NoiseParams&)> Strategy = Noise3D::Perlin;
	};

	constexpr static u32 Noise3DChannels = 4u;

	struct Noise3DWrapper
	{
		Noise3DChannel Channels[Noise3DChannels]{};
		AssetHandle<Texture3D> Texture = AssetManager::Put<Texture3D>(128u, Texture::CreationInfoFromData{ .CpuFormat = GL_RGBA, .GpuFormat = GL_RGBA32F });

		void Fill()
		{
			for (const auto& channel: Channels) channel.Strategy(Texture, channel.Params);
		}
	};
}