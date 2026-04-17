#include "resources/FileSystem.h"

#include <string>
#include <fstream>
#include <vector>
#include <stdexcept>

#define STB_IMAGE_IMPLEMENTATION
#include "stbimg/stb_image.h"

#include "spdlog/spdlog.h"


namespace EnGl
{
	std::vector<char> FileSystem::ReadFile(const std::filesystem::path& path)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);

		if (!file.is_open()) throw std::runtime_error("Could not open file at " + path.string());

		size_t fileSize = static_cast<size_t>(file.tellg());

		file.seekg(0);

		std::vector<char> res(fileSize);

		file.read(res.data(), fileSize);

		res.push_back('\0');
		return res;
	}

	void FileSystem::FreeImage(unsigned char* data)
	{
		stbi_image_free(data);
	}


	FileSystem::RaiiImageData FileSystem::ReadImage(const std::filesystem::path& path, int* width, int* height, int* nrChannels, int reqChannels, bool flip)
	{
		stbi_set_flip_vertically_on_load(flip);
		unsigned char* data = stbi_load(path.string().c_str(), width, height, nrChannels, reqChannels);

		if (*width <= 0 || *height <= 0 || !data)
		{
			const char* reason = stbi_failure_reason();
			spdlog::error("Failed to load {}: {}", path.string(), reason ? reason : "unknown error");
			throw std::runtime_error("Failed to load texture");
		}

		return data;
	}

	FileSystem::RaiiImageData::~RaiiImageData()
	{
		if (Data)
		{
			FreeImage(Data);
		}
	}
}