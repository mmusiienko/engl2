#pragma once
#include "../../resources/renderer/base/Texture.h"


namespace EnGl
{
	struct FractalNoiseParams
	{
		u32 NOctaves = 8;
		f32 persistence = 0.5f;
		f32 lacunarity = 2.0f;

		u32 ChannelFrom = 1;
		u32 ChannelTo = 1;
	};

	class FractalNoise2D : public Texture2D
	{
	public:
		FractalNoise2D(const FractalNoiseParams& fParams, const Texture2D& baseNoise);
		void Fill(const FractalNoiseParams& fParams, const Texture2D& baseNoise);
	};

	class FractalNoise3D : public Texture3D
	{
	public:
		FractalNoise3D(const FractalNoiseParams& fParams, const Texture3D& baseNoise);
		void Fill(const FractalNoiseParams& fParams, const Texture3D& baseNoise);
	};
}