#pragma once
#include "algorithm/compute/noise/NoiseTexture.h"


namespace EnGl
{
	class UiComponents
	{
	public:
		static bool Noise3DChannelView(Noise3DChannel& params);
		static void Noise3DView(Noise3DWrapper& wrapper);
		static bool Noise2DChannelView(Noise2DChannel& params);
		static void Noise2DView(Noise2DWrapper& wrapper);
		static void Texture2DView(AssetHandle<Texture2D> tex, glm::vec2 scale = glm::vec2{ 1.0f });
		static void Texture3DView(AssetHandle<Texture3D> tex);
		static bool InputUInt(const char* label, u32* v, int step = 1, int step_fast = 100);
	};
}