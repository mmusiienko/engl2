#include "../base/Cubemap.h"
#include "spdlog/spdlog.h"
#include "../resources/FileSystem.h"
#include "../core/Global.h"

namespace EnGl
{
	Cubemap::Cubemap(const std::vector<FaceData>& data)
	{

		GL_CHECK(glGenTextures(1, &m_Id));
		GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, m_Id));

		i32 i = 0;
		for (const auto& [data, width, height, nr_channels] : data)
		{
			GL_CHECK(glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_SRGB, width, height, 0,
				GL_RGB, GL_UNSIGNED_BYTE, data
			));
			i++;
		}

		GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
		GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		m_Props = { .CpuFormat = GL_RGB, .GpuFormat = GL_SRGB, .DataType = GL_UNSIGNED_BYTE, .Type = GL_TEXTURE_CUBE_MAP };
	}
}