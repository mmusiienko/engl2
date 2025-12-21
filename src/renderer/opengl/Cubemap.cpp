#include "../base/Cubemap.h"
#include "spdlog/spdlog.h"
#include "../resources/FileSystem.h"
#include "../core/Global.h"

namespace EnGl
{
	Cubemap::Cubemap(const std::filesystem::path& path)
	{
		spdlog::info("Loading cubemap: {}", path);
		int width, height, nr_channels;
		GL_CHECK(glGenTextures(1, &m_Id));
		GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, m_Id));

		unsigned char* data = FileSystem::ReadImageNoFlip(path, &width, &height, &nr_channels, 0);

		if (width <= 0 || height <= 0 || !data)
		{
			spdlog::error("Error loading cubemap texture with stbi: {}", path);
			FileSystem::FreeImage(data);
		}

		u32 faceWidth = width / 4;
		
		std::vector<std::vector<unsigned char>> faces
		{
			GetFace(data, 2 * faceWidth, faceWidth, faceWidth, nr_channels),
			GetFace(data, 0, faceWidth, faceWidth, nr_channels),
			GetFace(data, faceWidth, 0, faceWidth, nr_channels),
			GetFace(data, faceWidth, 2 * faceWidth, faceWidth, nr_channels),
			GetFace(data, faceWidth, faceWidth, faceWidth, nr_channels),
			GetFace(data, 3 * faceWidth, faceWidth, faceWidth, nr_channels),
		};

		for (u32 i = 0; i < 6; i++)
		{
			GL_CHECK(glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, faceWidth, faceWidth, 0,
				GL_RGB, GL_UNSIGNED_BYTE, faces[i].data()
			));
		}

		GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
		GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
		GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
		GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

		m_Props = { .CpuFormat = GL_RGB, .GpuFormat = GL_RGB, .DataType = GL_UNSIGNED_BYTE, .Type = GL_TEXTURE_CUBE_MAP };
	}

	std::vector<unsigned char> Cubemap::GetFace(unsigned char* data, u32 topLeftX, u32 topLeftY, u32 w, u32 nrchannels)
	{
		std::vector<unsigned char> faceData(w * w * nrchannels);

		for (u32 x = 0; x < w; x++)
		{
			for (u32 y = 0; y < w; y++)
			{
				for (u32 c = 0; c < nrchannels; c++)
				{
					faceData[(y * w + x) * nrchannels + c] =
						data[((topLeftY + y) * w * 4 + (topLeftX + x)) * nrchannels + c];
				}
			}
		}

		return faceData;
	}

	Cubemap::Cubemap(const std::filesystem::path& path, const std::vector<std::string>& faces)
	{
		spdlog::info("Loading cubemap: {}", path);
		int width, height, nr_channels;
		u32 i = 0;
		GL_CHECK(glGenTextures(1, &m_Id));
		GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, m_Id));

		for (const auto& face : faces)
		{
			auto facePath = std::filesystem::path(path) / face;
			unsigned char* data = FileSystem::ReadImageNoFlip(facePath, &width, &height, &nr_channels, 0);

			if (width <= 0 || height <= 0 || !data)
			{
				spdlog::error("Error loading cubemap texture with stbi: {}", facePath.string());
				FileSystem::FreeImage(data);
			}


			GL_CHECK(
				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
				)
			);

			i++;

			GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
			GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
			GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
			GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

			FileSystem::FreeImage(data);

			m_Props = { .CpuFormat = GL_RGB, .GpuFormat = GL_RGB, .DataType = GL_UNSIGNED_BYTE, .Type = GL_TEXTURE_CUBE_MAP };
		}
	}
}