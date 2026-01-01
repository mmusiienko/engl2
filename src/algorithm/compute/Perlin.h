#pragma once
#include "../math/Math.h"
#include "../resources/renderer/base/Texture.h"
#include "../resources/renderer/base/ComputeShader.h"
#include <vector>

namespace EnGl
{

	struct PerlinNoise2DParams
	{
		u32 w = 128; u32 h = 128; u32 NCells = 8; u32 NChannel = 1;
	};

	struct PerlinNoise2DParamsShort
	{
		u32 dim = 128; u32 NCells = 8; u32 NChannel = 1;

		operator PerlinNoise2DParams() const {
			return { .w = dim, .h = dim, .NCells = NCells, .NChannel = NChannel };
		}
	};

	struct PerlinNoise3DParams
	{
		u32 w = 128; u32 h = 128; u32 d = 128; u32 NCells = 8; u32 NChannel = 1;
	};

	struct PerlinNoise3DParamsShort
	{
		u32 dim = 128; u32 NCells = 8; u32 NChannel = 1;

		operator PerlinNoise3DParams() const {
			return { .w = dim, .h = dim, .d = dim, .NCells = NCells, .NChannel = NChannel };
		}
	};

	class PerlinNoise2D : public Texture2D
	{
	public:
		PerlinNoise2D(const PerlinNoise2DParams& params);
		void Fill(const PerlinNoise2DParams& params);
	private:
		void RunCompute(const std::vector<glm::vec2>& gradients, const PerlinNoise2DParams& params);
		static std::vector<glm::vec2> GenerateGradientsOnVertices(u32 NCells);
	};

	class PerlinNoise3D : public Texture3D
	{
	public:
		PerlinNoise3D(const PerlinNoise3DParams& params);
		void Fill(const PerlinNoise3DParams& params);
	private:
		void RunCompute(const std::vector<glm::vec4>& gradients, const PerlinNoise3DParams& params);
		static std::vector<glm::vec4> GenerateGradientsOnVertices(u32 NCells);
	};

	
}