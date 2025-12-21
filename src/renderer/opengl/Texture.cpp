#include "../base/Texture.h"
#include <glad/glad.h>

#include "../core/Core.h"


namespace EnGl
{
	Texture::~Texture()
	{
		GL_CHECK(glDeleteTextures(1, &m_Id));
	}

	void Texture::Bind() const
	{
		GL_CHECK(glBindTexture(Props().Type, m_Id));
	}

	void Texture::Create()
	{
		GL_CHECK(glGenTextures(1, &m_Id));
	}

	Texture2D::Texture2D(u32 d, const CreationInfoFromData& info)
	{
		CreateTexture2DFromData(d, d, info);
	}

	Texture2D::Texture2D(u32 w, u32 h, const CreationInfoFromData& info)
	{
		CreateTexture2DFromData(w, h, info);
	}

	void Texture2D::CreateTexture2DFromData(u32 w, u32 h, const CreationInfoFromData& info)
	{
		Create();

		m_Props = { 
			.CpuFormat = info.CpuFormat,
			.GpuFormat = info.GpuFormat,
			.DataType = info.DataType,
			.Type = GL_TEXTURE_2D,
			.Wrap = info.Common.Wrap,
			.Filtering = info.Common.Filtering,
			.w = w,
			.h = h
		};

		UpdateParameters(info.Data);
	}

	void Texture2D::UpdateParameters(const void* data)
	{
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_Id));

		GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, m_Props.GpuFormat, m_Props.w, m_Props.h, 0, m_Props.CpuFormat, m_Props.DataType, data));

		GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));

		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_Props.Wrap));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_Props.Wrap));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_Props.Filtering));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_Props.Filtering));

	}

	Texture3D::Texture3D(u32 d, const CreationInfoFromData& info)
	{
		CreateTexture3DFromData(d, d, d, info);
	}

	Texture3D::Texture3D(u32 w, u32 h, u32 d, const CreationInfoFromData& info)
	{
		CreateTexture3DFromData(w, h, d, info);
	}

	void Texture3D::CreateTexture3DFromData(u32 w, u32 h, u32 d , const CreationInfoFromData& info)
	{
		Create();

		m_Props = {
			.CpuFormat = info.CpuFormat,
			.GpuFormat = info.GpuFormat,
			.DataType = info.DataType,
			.Type = GL_TEXTURE_2D,
			.Wrap = info.Common.Wrap,
			.Filtering = info.Common.Filtering,
			.w = w,
			.h = h,
			.d = d
		};

		UpdateParameters(info.Data);
	}

	void Texture3D::UpdateParameters(const void* data)
	{
		GL_CHECK(glGenTextures(1, &m_Id));
		GL_CHECK(glBindTexture(GL_TEXTURE_3D, m_Id));

		GL_CHECK(glTexImage3D(GL_TEXTURE_3D, 0, m_Props.GpuFormat, m_Props.w, m_Props.h, m_Props.d, 0, m_Props.CpuFormat, m_Props.DataType, data));

		GL_CHECK(glGenerateMipmap(GL_TEXTURE_3D));

		GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, m_Props.Wrap));
		GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, m_Props.Wrap));
		GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, m_Props.Wrap));
		GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, m_Props.Filtering));
		GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, m_Props.Filtering));
	}
}

