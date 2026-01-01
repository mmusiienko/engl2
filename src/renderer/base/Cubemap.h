#pragma once
#include <filesystem>
#include "../core/Core.h"
#include "Texture.h"

namespace EnGl
{
	class Cubemap : public Texture
	{
	public:
		struct FaceData
		{
			unsigned char* Data;
			i32 Width;
			i32 Height;
			i32 NChannels;
		};

		Cubemap(const std::vector<FaceData>& data);
		Cubemap(Cubemap&& other) noexcept = default;
		Cubemap& operator=(Cubemap&& other) noexcept = default;
	};
}