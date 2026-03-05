#pragma once
#include "../math/Math.h"
#include "../renderer/base/Texture.h"
#include "../renderer/base/ComputeShader.h"
#include <vector>
#include "../resources/importers/AssetManager.h"


namespace EnGl
{
	struct PerlinNoiseParams
	{
		u32 NCells = 8; u32 NChannel = 1;
	};

	class PerlinNoise2D
	{
	public:
		static void Fill(AssetHandle<Texture2D> texture, const PerlinNoiseParams& params);
	private:
		static std::vector<glm::vec2> GenerateGradientsOnVertices(u32 NCells);
	};

	class PerlinNoise3D
	{
	public:
		static void Fill(AssetHandle<Texture3D> texture, const PerlinNoiseParams& params);
	private:
		static std::vector<glm::vec4> GenerateGradientsOnVertices(u32 NCells);
	};
}