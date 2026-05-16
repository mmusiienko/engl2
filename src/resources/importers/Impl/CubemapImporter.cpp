#include "resources/importers/AssetImporter.h"

#include <vector>

#include "resources/FileSystem.h"


namespace EnGl
{
	static std::vector<unsigned char> GetFace(unsigned char* data, u32 topLeftX, u32 topLeftY, u32 w, u32 nrchannels);

	Cubemap AssetImporter<Cubemap>::Import(const Params& params)
	{
		spdlog::info("Loading cubemap at: {}", params.Path.string());

		std::vector<Cubemap::FaceData> facesRaw;
		facesRaw.reserve(6);

		if (!params.Faces.empty())
		{
			std::vector<FileSystem::RaiiImageData> data;
			data.reserve(6);

			for (const auto& [entry, flip] : std::views::zip(params.Faces, params.Flip))
			{
				i32 width, height, nr_channels;

				auto imgData = FileSystem::ReadImage(entry, &width, &height, &nr_channels, 0, flip);
				facesRaw.emplace_back(imgData.Data, width, height, nr_channels);
				data.push_back(std::move(imgData));
			}

			return Cubemap{ facesRaw };
		}
		else
		{
			i32 width, height, nr_channels;
			auto data = FileSystem::ReadImage(params.Path, &width, &height, &nr_channels, 0);

			if (width <= 0 || height <= 0 || !data.Data)
			{
				spdlog::error("Error loading cubemap texture with stbi: {}", params.Path.string());
				throw std::runtime_error("Error loading cubemap texture with stbi: " + params.Path.string());
			}

			u32 faceWidth = width / 4;

			std::vector<std::vector<unsigned char>> faces
			{
				GetFace(data.Data, 2 * faceWidth, faceWidth, faceWidth, nr_channels),
				GetFace(data.Data, 0, faceWidth, faceWidth, nr_channels),
				GetFace(data.Data, faceWidth, 0, faceWidth, nr_channels),
				GetFace(data.Data, faceWidth, 2 * faceWidth, faceWidth, nr_channels),
				GetFace(data.Data, faceWidth, faceWidth, faceWidth, nr_channels),
				GetFace(data.Data, 3 * faceWidth, faceWidth, faceWidth, nr_channels),
			};

			for (size_t i = 0; i < 6; i++)
			{
				facesRaw.emplace_back(faces[i].data(), faceWidth, faceWidth, nr_channels);
			}

			return Cubemap{ facesRaw };
		}
	}

	static std::vector<unsigned char> GetFace(unsigned char* data, u32 topLeftX, u32 topLeftY, u32 w, u32 nrchannels)
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
}