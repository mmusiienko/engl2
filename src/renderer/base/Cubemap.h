#pragma once
#include <filesystem>


namespace EnGl
{
	class Cubemap : public Texture
	{
	public:
		Cubemap(const std::filesystem::path& path, const std::vector<std::string>& faces);
		Cubemap(const std::filesystem::path& path);
		Cubemap(Cubemap&& other) noexcept = default;
		Cubemap& operator=(Cubemap&& other) noexcept = default;
	private:
		static std::vector<unsigned char> GetFace(unsigned char* data, u32 topLeftX, u32 topLeftY, u32 w, u32 nrchannels);
	};
}