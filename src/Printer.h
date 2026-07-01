#pragma once

#include "math/Math.h"


namespace EnGl
{
	class Printer
	{
	public:
		static void Print(const glm::vec3& val);
		static void Print(const glm::quat& val);
		static void Print(const glm::mat4& val);
		static void Print(const glm::mat3x4& val);
	};
}