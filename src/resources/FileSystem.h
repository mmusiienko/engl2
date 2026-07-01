#pragma once
#include <string>
#include <vector>
#include <filesystem>

#include "core/Core.h"

namespace EnGl
{
	class FileSystem
	{
	public:
		struct RaiiImageData
		{
			unsigned char* Data;

			RaiiImageData(unsigned char* data = nullptr) : Data(data) {}
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
		static RaiiImageData ReadImageFromMemory(
			const std::string& name,
			unsigned char* embeddedData,
			i32 embeddedLength,
			i32* width, i32* height, i32* nrChannels,
			i32 reqChannels, bool flip
		);
		static RaiiImageData ReadImage(const std::filesystem::path& path, i32* width, i32* height, i32* nrChannels, i32 reqChannels, bool flip = false);
		static void FreeImage(unsigned char* data);
	};
}