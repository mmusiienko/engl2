#include "../AssetImporter.h"

#include <vector>
#include "../../FileSystem.h"


namespace EnGl
{
	Texture2D AssetImporter<Texture2D>::Import(const Params& params)
	{
		static const GLenum COMPONENTS_TO_CHANNELS[4]{ GL_RED, GL_RG, GL_RGB, GL_RGBA };
		static const GLenum COMPONENTS_TO_CHANNELS_GPU[4]{ GL_RED, GL_RG, GL_SRGB, GL_SRGB_ALPHA };

		auto pathStr = params.Path.string();
		spdlog::info("Loading texture: {}", pathStr);
		int width, height, nr_channels;

		auto data = FileSystem::ReadImage(pathStr.c_str(), &width, &height, &nr_channels, 0, params.Flip);

		auto format = COMPONENTS_TO_CHANNELS[nr_channels - 1];
		auto gpuformat = COMPONENTS_TO_CHANNELS_GPU[nr_channels - 1];
		
		Texture2D tex{ 
			static_cast<u32>(width), static_cast<u32>(height), 
			Texture::CreationInfoFromData {
				.Data = data.Data,
				.CpuFormat = format,
				.GpuFormat = gpuformat,
				.DataType = GL_UNSIGNED_BYTE,
				.Common = params.TextureParams
			} 
		};

		return tex;
	}
}