#pragma once

#include "Core.h"
#include "../resources/StaticModel.h"
#include "../resources/importers/AssetManager.h"

namespace EnGl
{
	class Global
	{
	public:
		static f32 DeltaTime;
		static f32 LastFrame;
		static f32 Time;
		static glm::uvec2 WindowResolution;
		static bool IsPaused;

		static void ShutDown();
	};
}
