#include <string>
#include <fstream>
#include <vector>
#include <stdexcept>
#include "FileSystem.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stbimg/stb_image.h"


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

	unsigned char* FileSystem::ReadImage(const std::filesystem::path& path, int* width, int* height, int* nrChannels, int reqChannels)
	{
		stbi_set_flip_vertically_on_load(true);
		return stbi_load(path.string().c_str(), width, height, nrChannels, reqChannels);
	}

	unsigned char* FileSystem::ReadImageNoFlip(const std::filesystem::path& path, int* width, int* height, int* nrChannels, int reqChannels)
	{
		stbi_set_flip_vertically_on_load(false);
		return stbi_load(path.string().c_str(), width, height, nrChannels, reqChannels);
	}

	void FileSystem::FreeImage(unsigned char* data)
	{
		stbi_image_free(data);
	}
}