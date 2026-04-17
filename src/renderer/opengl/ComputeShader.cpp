#include "../base/ComputeShader.h"


namespace EnGl
{
	ComputeShader::ComputeShader(const std::vector<ShaderUnit>& shaderUnits): Shader(shaderUnits) {}

	void ComputeShader::Dispatch(const ComputeInfo& info)
	{
		GL_CHECK(glDispatchCompute(info.GroupSizeX, info.GroupSizeY, info.GroupSizeZ));
	}

	void ComputeShader::Wait(u32 bit)
	{
		GL_CHECK(glMemoryBarrier(bit));
	}

	void ComputeShader::DispatchWait(const ComputeInfo& info, u32 bit)
	{
		Dispatch(info);
		Wait(bit);
	}

	void ComputeShader::BindReadTexture(const Texture2D& tex, u32 unit)
	{
		GL_CHECK(glBindImageTexture(unit, tex.Id(), 0, GL_FALSE, 0, GL_READ_ONLY, tex.Properties().GpuFormat));
	}

	void ComputeShader::BindWriteTexture(const Texture2D& tex, u32 unit)
	{
		GL_CHECK(glBindImageTexture(unit, tex.Id(), 0, GL_FALSE, 0, GL_WRITE_ONLY, tex.Properties().GpuFormat));
	}

	void ComputeShader::BindReadWriteTexture(const Texture2D& tex, u32 unit)
	{
		GL_CHECK(glBindImageTexture(unit, tex.Id(), 0, GL_FALSE, 0, GL_READ_WRITE, tex.Properties().GpuFormat));
	}

	void ComputeShader::BindReadTexture(const Texture3D& tex, u32 unit)
	{
		GL_CHECK(glBindImageTexture(unit, tex.Id(), 0, GL_TRUE, 0, GL_READ_ONLY, tex.Properties().GpuFormat));
	}

	void ComputeShader::BindWriteTexture(const Texture3D& tex, u32 unit)
	{
		GL_CHECK(glBindImageTexture(unit, tex.Id(), 0, GL_TRUE, 0, GL_WRITE_ONLY, tex.Properties().GpuFormat));
	}

	void ComputeShader::BindReadWriteTexture(const Texture3D& tex, u32 unit)
	{
		GL_CHECK(glBindImageTexture(unit, tex.Id(), 0, GL_TRUE, 0, GL_READ_WRITE, tex.Properties().GpuFormat));
	}
}

