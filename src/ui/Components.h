#pragma once
#include "../algorithm/compute/noise/Fractal.h"
#include "../algorithm/compute/noise/Perlin.h"
#include "../algorithm/compute/noise/Voronoi.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


namespace EnGl
{
	class UiComponents
	{
	public:
		struct ParamsChanged
		{
			bool BaseNoise = false;
			bool Fbm = false;
		};

		static ParamsChanged VoronoiNoiseChannelView(VoronoiNoiseParams& params, FractalNoiseParams& fParams);
		static ParamsChanged PerlinNoiseChannelView(PerlinNoiseParams& params, FractalNoiseParams& fParams);

		static void VoronoiNoiseView(VoronoiWrapper& wrapper);
		static void PerlinNoiseView(PerlinWrapper& wrapper);
		static bool InputUInt(const char* label, uint32_t* v, int step = 1, int step_fast = 100);
	};
}