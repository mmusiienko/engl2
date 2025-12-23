#pragma once

#include "../core/Core.h"
#include "Resource.h"


namespace EnGl
{
	class Texture : public Resource
	{
	public:

		struct CommonInfo
		{
			u32 Wrap = GL_REPEAT;
			u32 Filtering = GL_LINEAR;

			auto operator<=>(const CommonInfo&) const = default;
		};

		struct CreationInfoFromData
		{
			const void* Data = nullptr;

			u32 CpuFormat = GL_RGBA;
			u32 GpuFormat = GL_RGBA32F;
			u32 DataType = GL_FLOAT;

			CommonInfo Common{};
		};

		struct Props
		{
			u32 CpuFormat = GL_RGBA;
			u32 GpuFormat = GL_RGBA32F;
			u32 DataType = GL_FLOAT;
			u32 Type = GL_TEXTURE_2D;
			u32 Wrap = GL_REPEAT;
			u32 Filtering = GL_LINEAR;
			u32 w = 1;
			u32 h = 1;
			u32 d = 0;
		};

		Texture(Texture&& other) noexcept = default;
		Texture& operator=(Texture&& other) noexcept = default;
		virtual ~Texture();

		inline const Props& Properties() const
		{
			return m_Props;
		};

		inline Props& Properties()
		{
			return m_Props;
		};

		void Bind() const;
	protected:
		Texture() = default;
		void Create();

		Props m_Props{};
	};

	class Texture2D : public Texture
	{
	public:
		Texture2D(u32 d, const CreationInfoFromData& info = {});
		Texture2D(u32 w, u32 h, const CreationInfoFromData& info = {});
		Texture2D(Texture2D&& other) noexcept = default;
		Texture2D& operator=(Texture2D&& other) noexcept = default;
		void UpdateParameters(const void* data = nullptr);

	protected:
		void CreateTexture2DFromData(u32 w, u32 h, const CreationInfoFromData& info);
	};

	class Texture3D : public Texture
	{
	public:
		Texture3D(u32 d, const CreationInfoFromData& info = {});
		Texture3D(u32 w, u32 h, u32 d, const CreationInfoFromData& info = {});
		Texture3D(Texture3D&& other) noexcept = default;
		Texture3D& operator=(Texture3D&& other) noexcept = default;
		void UpdateParameters(const void* data = nullptr);

	protected:
		void CreateTexture3DFromData(u32 w, u32 h, u32 d, const CreationInfoFromData& info);
	};
}

namespace std
{
	template<>
	struct hash<EnGl::Texture::CommonInfo>
	{
		size_t operator()(const EnGl::Texture::CommonInfo& info) const
		{
			size_t res = 0;
			EnGl::hash_combine(res, info.Wrap);
			EnGl::hash_combine(res, info.Filtering);

			return res;
		}
	};
}