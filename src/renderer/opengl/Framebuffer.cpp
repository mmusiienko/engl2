#include "../base/Framebuffer.h"
#include "../../core/Global.h"


namespace EnGl
{
	Framebuffer::Framebuffer(u32 w, u32 h) : 
		m_Resolution({w, h}),
		m_Depth(
			AssetManager::Put<Texture2D>(Texture2D(w, h, Texture::CreationInfoFromData{
				.CpuFormat = GL_DEPTH_COMPONENT,
				.GpuFormat = GL_DEPTH_COMPONENT32F,
				.DataType = GL_FLOAT
				}))
		),
		m_Color(
			AssetManager::Put<Texture2D>(Texture2D{ w, h, Texture::CreationInfoFromData{
				.CpuFormat = GL_RGBA,
				.GpuFormat = GL_RGBA16F,
				.DataType = GL_FLOAT,
			}})
		),
		m_Depth2(
			AssetManager::Put<Texture2D>(Texture2D(w, h, Texture::CreationInfoFromData{
				.CpuFormat = GL_DEPTH_COMPONENT,
				.GpuFormat = GL_DEPTH_COMPONENT32F,
				.DataType = GL_FLOAT
				}))
		),
		m_Color2(
			AssetManager::Put<Texture2D>(Texture2D{ w, h, Texture::CreationInfoFromData{
				.CpuFormat = GL_RGBA,
				.GpuFormat = GL_RGBA16F,
				.DataType = GL_FLOAT,
			} })
			)
	{
		GL_CHECK(glGenFramebuffers(1, &m_Id));
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_Id));

		auto [color, g1] = AssetManager::GetAsset(m_Color);
		auto [depth, g2] = AssetManager::GetAsset(m_Depth);
		
		assert(color && depth);

		GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color->Id(), 0));

		GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth->Id(), 0));

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			throw std::runtime_error("framebuffer is not complete");

		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	void Framebuffer::Bind() const
	{
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_Id));
	}

	void Framebuffer::Swap()
	{
		std::swap(m_Color, m_Color2);
		std::swap(m_Depth, m_Depth2);

		auto [color, g1] = AssetManager::GetAsset(m_Color);
		auto [depth, g2] = AssetManager::GetAsset(m_Depth);

		assert(color && depth);

		Bind();

		GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color->Id(), 0));
		GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth->Id(), 0));
	}

	void Framebuffer::Resize(u32 w, u32 h)
	{
		auto [color, g1] = AssetManager::GetAsset(m_Color);
		auto [depth, g2] = AssetManager::GetAsset(m_Depth);
		auto [color2, g3] = AssetManager::GetAsset(m_Color2);
		auto [depth2, g4] = AssetManager::GetAsset(m_Depth2);

		assert(color && depth);

		color->Properties().w = w;
		color->Properties().h = h;
		color->UpdateParameters();
		color->Update();

		depth->Properties().w = w;
		depth->Properties().h = h;
		depth->UpdateParameters();
		depth->Update();

		color2->Properties().w = w;
		color2->Properties().h = h;
		color2->UpdateParameters();
		color2->Update();

		depth2->Properties().w = w;
		depth2->Properties().h = h;
		depth2->UpdateParameters();
		depth2->Update();

		Bind();

		GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color->Id(), 0));
		GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth->Id(), 0));
		m_Resolution = { w, h };
	}

	void Framebuffer::Unbind()
	{
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	Framebuffer::~Framebuffer()
	{
		GL_CHECK(glDeleteFramebuffers(1, &m_Id));
	}
}

