#pragma once
#include "../entity/GPUData.h"
#include "../algorithm/compute/Fractal.h"
#include "../algorithm/compute/Perlin.h"
#include "../algorithm/compute/Voronoi.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


namespace EnGl
{
	struct VoronoiWrapper
	{
		VoronoiNoise3DParamsShort ParamsR{ .NPoints= 2, .NChannel = 1 };
		FractalNoiseParams FParamsR{ .ChannelFrom = 1, .ChannelTo = 1 };
		VoronoiNoise3DParamsShort ParamsG{ .NPoints = 4, .NChannel = 2 };
		FractalNoiseParams FParamsG{ .ChannelFrom = 2, .ChannelTo = 2 };
		VoronoiNoise3DParamsShort ParamsB{ .NPoints = 6, .NChannel = 3 };
		FractalNoiseParams FParamsB{ .ChannelFrom = 3, .ChannelTo = 3 };
		VoronoiNoise3DParamsShort ParamsA{ .NPoints = 8, .NChannel = 4 };
		FractalNoiseParams FParamsA{ .ChannelFrom = 4, .ChannelTo = 4 };

		ref<VoronoiNoise3D> Voronoi = make_ref<VoronoiNoise3D>(ParamsR);
		ref<FractalNoise3D> VoronoiFBM = make_ref<FractalNoise3D>(FParamsR, *Voronoi);

		u32 Dim = ParamsR.dim;

		VoronoiWrapper()
		{
			Voronoi->Fill(ParamsG);
			Voronoi->Fill(ParamsB);
			Voronoi->Fill(ParamsA);
			VoronoiFBM->Fill(FParamsG, *Voronoi);
			VoronoiFBM->Fill(FParamsB, *Voronoi);
			VoronoiFBM->Fill(FParamsA, *Voronoi);
		}
	};

	struct PerlinWrapper
	{
		PerlinNoise3DParamsShort ParamsR{ .NCells=4, .NChannel = 1 };
		FractalNoiseParams FParamsR{ .ChannelFrom = 1, .ChannelTo = 1 };
		PerlinNoise3DParamsShort ParamsG{ .NCells = 16, .NChannel = 2 };
		FractalNoiseParams FParamsG{ .ChannelFrom = 2, .ChannelTo = 2 };
		PerlinNoise3DParamsShort ParamsB{ .NCells = 32,.NChannel = 3 };
		FractalNoiseParams FParamsB{ .ChannelFrom = 3, .ChannelTo = 3 };
		PerlinNoise3DParamsShort ParamsA{ .NCells = 64, .NChannel = 4 };
		FractalNoiseParams FParamsA{ .ChannelFrom = 4, .ChannelTo = 4 };

		ref<PerlinNoise3D> Perlin = make_ref<PerlinNoise3D>(ParamsR);
		ref<FractalNoise3D> PerlinFBM = make_ref<FractalNoise3D>(FParamsR, *Perlin);

		u32 Dim = ParamsR.dim;

		PerlinWrapper()
		{
			Perlin->Fill(ParamsG);
			Perlin->Fill(ParamsB);
			Perlin->Fill(ParamsA);
			PerlinFBM->Fill(FParamsG, *Perlin);
			PerlinFBM->Fill(FParamsB, *Perlin);
			PerlinFBM->Fill(FParamsA, *Perlin);
		}
	};

	class UiComponents
	{
	public:
		struct ParamsChanged
		{
			bool BaseNoise = false;
			bool Fbm = false;
		};

		static ParamsChanged VoronoiNoiseChannelView(VoronoiNoise3DParamsShort& params, FractalNoiseParams& fParams, u32 id);
		static ParamsChanged PerlinNoiseChannelView(PerlinNoise3DParamsShort& params, FractalNoiseParams& fParams, u32 id);

		static void VoronoiNoiseView(VoronoiWrapper& wrapper);
		static void PerlinNoiseView(PerlinWrapper& wrapper);
		static void AttenuationView(Attenuation& attenuation);
		static void Vec3View(const std::string& title, glm::vec3& vec);
		static void ColorView(Color& color);
		static bool InputUInt(const char* label, uint32_t* v, int step = 1, int step_fast = 100);
	};
}