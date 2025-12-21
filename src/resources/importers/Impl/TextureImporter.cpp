#include "../AssetImporter.h"

#include <vector>
#include "../../FileSystem.h"


namespace EnGl
{
	Texture2D AssetImporter<Texture2D>::Import(const Params& params)
	{
		static const GLenum COMPONENTS_TO_CHANNELS[4]{ GL_RED, GL_RG, GL_RGB, GL_RGBA };

		auto pathStr = params.Path.string();
		spdlog::info("Loading texture: {}", pathStr);
		int width, height, nr_channels;

		unsigned char* data = FileSystem::ReadImageNoFlip(pathStr.c_str(), &width, &height, &nr_channels, 0);


		if (width <= 0 || height <= 0 || !data)
		{
			spdlog::error("Error loading texture with stbi: {}", pathStr);
			FileSystem::FreeImage(data);
			throw std::runtime_error("Error loading texture with stbi: " + pathStr);
		}
		auto format = COMPONENTS_TO_CHANNELS[nr_channels - 1];
		
		Texture2D tex{ 
			static_cast<u32>(width), static_cast<u32>(height), 
			Texture::CreationInfoFromData {
				.Data = data,
				.CpuFormat = format,
				.GpuFormat = format,
				.DataType = GL_UNSIGNED_BYTE,
				.Common = params.TextureParams
			} 
		};

		FileSystem::FreeImage(data);

		return tex;
	}
}