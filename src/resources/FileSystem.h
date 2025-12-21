#pragma once
#include <string>
#include <vector>
#include <filesystem>


namespace EnGl
{
	class FileSystem
	{
	public:
		static std::vector<char> ReadFile(const std::filesystem::path& path);
		static unsigned char* ReadImage(const std::filesystem::path& path, int* width, int* height, int* nrChannels, int reqChannels);
		static unsigned char* ReadImageNoFlip(const std::filesystem::path& path, int* width, int* height, int* nrChannels, int reqChannels);
		static void FreeImage(unsigned char* data);
	};
}