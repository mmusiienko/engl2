#pragma once

#include "Core.h"
#include "../math/Math.h"


namespace EnGl
{
	class Texture2D;

	class Global
	{
	public:
		static f32 DeltaTime;
		static f32 LastFrame;
		static f32 Time;
		static glm::uvec2 WindowResolution;
		static bool IsPaused;

		static void StartUp();
		static void ShutDown();
	};
}
