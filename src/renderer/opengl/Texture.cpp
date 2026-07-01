#include "renderer/base/Texture.h"

#include "core/Core.h"


namespace EnGl
{
	Texture::~Texture()
	{
		GL_CHECK(glDeleteTextures(1, &m_Id));
	}

	void Texture::Bind() const
	{
		GL_CHECK(glBindTexture(m_Props.Type, m_Id));
	}

	void Texture::GenerateMips()
	{
		Bind();
		GL_CHECK(glGenerateMipmap(m_Props.Type));
	}

	void Texture::Create()
	{
		GL_CHECK(glGenTextures(1, &m_Id));
	}

	Texture2D::Texture2D(const Texture2D& other)
	{
		Create();

		m_Props = other.m_Props;

		Update();
		UpdateParameters();

		glCopyImageSubData(
			other.m_Id,
			GL_TEXTURE_2D,
			0,
			0, 0, 0,
			m_Id,
			GL_TEXTURE_2D,
			0, 
			0, 0, 0,
			m_Props.w, m_Props.h, 1
		);

		GenerateMips();
	}

	Texture2D& Texture2D::operator=(const Texture2D& other)
	{
		if (m_Id == other.m_Id)
			return *this;

		m_Props = other.m_Props;


		Update();
		UpdateParameters();

		glCopyImageSubData(
			other.m_Id,
			GL_TEXTURE_2D,
			0,
			0, 0, 0,
			m_Id,
			GL_TEXTURE_2D,
			0,
			0, 0, 0,
			m_Props.w, m_Props.h, 1
		);

		GenerateMips();

		return *this;
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
			.MinFilter = info.Common.MinFilter,
			.MagFilter = info.Common.MagFilter,
			.w = w,
			.h = h,
			.BorderColor = info.Common.BorderColor
		};

		Update(info.Data);
		UpdateParameters();

		GenerateMips();
	}

	void Texture2D::Update(const void* data)
	{
		Bind();

		GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, m_Props.GpuFormat, m_Props.w, m_Props.h, 0, m_Props.CpuFormat, m_Props.DataType, data));
	}

	void Texture2D::UpdateParameters()
	{
		Bind();

		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, m_Props.Wrap));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, m_Props.Wrap));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_Props.MinFilter));
		GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_Props.MagFilter));
		GL_CHECK(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(m_Props.BorderColor)));
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
			.Type = GL_TEXTURE_3D,
			.Wrap = info.Common.Wrap,
			.MinFilter = info.Common.MinFilter,
			.MagFilter = info.Common.MagFilter,
			.w = w,
			.h = h,
			.d = d,
			.BorderColor = info.Common.BorderColor
		};

		Update(info.Data);
		UpdateParameters();

		GenerateMips();
	}

	void Texture3D::Update(const void* data)
	{
		Bind();

		GL_CHECK(glTexImage3D(GL_TEXTURE_3D, 0, m_Props.GpuFormat, m_Props.w, m_Props.h, m_Props.d, 0, m_Props.CpuFormat, m_Props.DataType, data));
	}

	void Texture3D::UpdateParameters()
	{
		Bind();

		GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, m_Props.Wrap));
		GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, m_Props.Wrap));
		GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, m_Props.Wrap));
		GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, m_Props.MinFilter));
		GL_CHECK(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, m_Props.MagFilter));
		GL_CHECK(glTexParameterfv(GL_TEXTURE_3D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(m_Props.BorderColor)));
	}
}

