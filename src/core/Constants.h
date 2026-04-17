#pragma once

#include "../math/Math.h"

namespace EnGl
{
	namespace Constants
	{
		constexpr inline glm::vec3 FORWARD   { 0.0f, 0.0f, -1.0f };
		constexpr inline glm::vec3 UP        { 0.0f, 1.0f, 0.0f };
		constexpr inline glm::vec3 RIGHT     { 1.0f, 0.0f, 0.0f };
		constexpr inline f32 TOLERANCE = 0.001f;
	}
}