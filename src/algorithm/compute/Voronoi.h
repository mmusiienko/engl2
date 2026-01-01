#pragma once
#include "../math/Math.h"
#include "../resources/renderer/base/Texture.h"
#include "../resources/renderer/base/ComputeShader.h"
#include <vector>

namespace EnGl
{
	struct VoronoiNoise2DParams
	{
		u32 w = 128; u32 h = 128; u32 NPoints = 8; u32 NChannel = 1;
	};
	struct VoronoiNoise3DParams
	{
		u32 w = 128; u32 h = 128; u32 d = 128; u32 NPoints = 8; u32 NChannel = 1;
	};
	struct VoronoiNoise2DParamsShort
	{
		u32 dim = 128; u32 NPoints = 8; u32 NChannel = 1;

		operator VoronoiNoise2DParams() const {
			return { .w = dim, .h = dim, .NPoints = NPoints, .NChannel=NChannel };
		}
	};
	struct VoronoiNoise3DParamsShort
	{
		u32 dim = 128; u32 NPoints = 2; u32 NChannel = 1;

		operator VoronoiNoise3DParams() const {
			return { .w = dim, .h = dim, .d = dim, .NPoints = NPoints, .NChannel=NChannel };
		}
	};

	class VoronoiNoise2D : public Texture2D
	{
	public:
		VoronoiNoise2D(const VoronoiNoise2DParams& params);

		void Fill(const VoronoiNoise2DParams& params);
	private:
		void RunCompute(const std::vector<glm::vec2>& points, const VoronoiNoise2DParams& params);
		static std::vector<glm::vec2> GeneratePointsWithinBounds(const VoronoiNoise2DParams& params);
	};

	class VoronoiNoise3D : public Texture3D
	{
	public:
		VoronoiNoise3D(const VoronoiNoise3DParams& params);

		void Fill(const VoronoiNoise3DParams& params);
	private:
		void RunCompute(const std::vector<glm::vec4>& points, const VoronoiNoise3DParams& params);
		static std::vector<glm::vec4> GeneratePointsWithinBounds(const VoronoiNoise3DParams& params);
	};
}