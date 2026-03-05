#pragma once
#include "../math/Math.h"
#include "../renderer/base/Texture.h"
#include "../renderer/base/ComputeShader.h"
#include "../resources/importers/AssetManager.h"
#include <vector>

namespace EnGl
{
	struct VoronoiNoiseParams
	{
		u32 NPoints = 8; u32 NChannel = 1;
	};

	class VoronoiNoise2D
	{
	public:
		static void Fill(AssetHandle<Texture2D> texture, const VoronoiNoiseParams& params);
	private:
		static std::vector<glm::vec2> GeneratePointsWithinBounds(const Texture2D::Props& props, const VoronoiNoiseParams& params);
	};

	class VoronoiNoise3D
	{
	public:
		static void Fill(AssetHandle<Texture3D> texture, const VoronoiNoiseParams& params);
	private:
		static std::vector<glm::vec4> GeneratePointsWithinBounds(const Texture2D::Props& props, const VoronoiNoiseParams& params);
	};
}