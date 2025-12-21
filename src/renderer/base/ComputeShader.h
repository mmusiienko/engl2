#pragma once

#include "Shader.h"
#include "Texture.h"


namespace EnGl
{
	class ComputeShader : public Shader
	{
	public:
		ComputeShader(const std::vector<ShaderUnit>& units);
		ComputeShader(ComputeShader&& other) noexcept = default;
		ComputeShader& operator=(ComputeShader&& other) noexcept = default;

		struct ComputeInfo
		{
			u32 GroupSizeX = 1;
			u32 GroupSizeY = 1;
			u32 GroupSizeZ = 1;
		};

		void Dispatch(const ComputeInfo& info);
		void Wait(u32 bit);
		void DispatchWait(const ComputeInfo& info, u32 bit);

		static void BindReadTexture(const Texture& tex, u32 unit);
		static void BindWriteTexture(const Texture& tex, u32 unit);
		static void BindReadWriteTexture(const Texture& tex, u32 unit);
	};
}