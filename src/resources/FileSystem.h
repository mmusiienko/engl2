#pragma once
#include <string>
#include <vector>
#include <filesystem>


namespace EnGl
{
	class FileSystem
	{
	public:
		struct RaiiImageData
		{
			unsigned char* Data;

			RaiiImageData(unsigned char* data) : Data(data) {}
			RaiiImageData(const RaiiImageData& other) = delete;
			RaiiImageData(RaiiImageData&& other) noexcept : Data(other.Data)
			{
				other.Data = nullptr;
			}
			RaiiImageData& operator=(const RaiiImageData& other) = delete;
			RaiiImageData& operator=(RaiiImageData&& other) noexcept
			{
				std::swap(Data, other.Data);
				return *this;
			}

			~RaiiImageData();
		};

		static std::vector<char> ReadFile(const std::filesystem::path& path);
		static RaiiImageData ReadImage(const std::filesystem::path& path, int* width, int* height, int* nrChannels, int reqChannels, bool flip = false);
		static void FreeImage(unsigned char* data);
	};
}