#include "resources/importers/AssetImporter.h"

#include <vector>

#include "resources/FileSystem.h"


namespace EnGl
{
	Texture2D AssetImporter<Texture2D>::Import(const Params& params)
	{
		static const GLenum COMPONENTS_TO_CHANNELS[4]{ GL_RED, GL_RG, GL_RGB, GL_RGBA };
		static const GLenum COMPONENTS_TO_CHANNELS_GPU_COLOR[4]{ GL_R8, GL_RG16F, GL_SRGB8, GL_SRGB8_ALPHA8 };
		static const GLenum COMPONENTS_TO_CHANNELS_GPU[4]{ GL_R8, GL_RG8, GL_RGB8, GL_RGBA8 };

		auto pathStr = params.Path.string();
		spdlog::info("Loading texture: {}", pathStr);
		int width, height, nr_channels;

		auto data = FileSystem::ReadImage(pathStr.c_str(), &width, &height, &nr_channels, 0, params.Flip);

		auto format = COMPONENTS_TO_CHANNELS[nr_channels - 1];
		auto gpuformat = params.IsColor ? COMPONENTS_TO_CHANNELS_GPU_COLOR[nr_channels - 1] : COMPONENTS_TO_CHANNELS_GPU[nr_channels - 1];
		
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