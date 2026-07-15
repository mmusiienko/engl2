#include "renderer/base/Framebuffer.h"
#include "core/Global.h"


namespace EnGl
{
	Framebuffer::Framebuffer(CreationInfo info)
	{
		GL_CHECK(glGenFramebuffers(1, &m_Id));
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_Id));

		assert(info.ColorAttachments.size() <= GL_MAX_COLOR_ATTACHMENTS && "Too many color attachments.");

		auto depth = AssetManager::GetAsset(info.DepthAttachment).Asset;
		if (depth)
		{
			GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth->Properties().Type, depth->Id(), 0));
			m_Resolution = { depth->Properties().w, depth->Properties().h };
		}
		else
		{
			assert(info.ColorAttachments.size() > 0 && "At least one attachment for a framebuffer has to exist.");

			auto color = AssetManager::GetAsset(info.ColorAttachments[0]).Asset;
			m_Resolution = { color->Properties().w, color->Properties().h };
		}

		u32 i = 0;
		for (const auto& colorA : info.ColorAttachments)
		{
			auto color = AssetManager::GetAsset(colorA).Asset;

			assert(color && "Color attachment is not loaded.");

			GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, color->Properties().Type, color->Id(), 0));

			i++;
		}

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			throw std::runtime_error("framebuffer is not complete");

		m_Color = std::move(info.ColorAttachments);
		m_Depth = info.DepthAttachment;

		if (depth)

		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	void Framebuffer::Bind() const
	{
		GL_CHECK(glViewport(0, 0, m_Resolution.x, m_Resolution.y));
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_Id));
	}

	void Framebuffer::DoubleBuffer()
	{
		m_Color2.clear();

		u32 i = 0;
		for (const auto& colorA : m_Color)
		{
			auto color = AssetManager::GetAsset(colorA).Asset;

			if (!color)
				throw std::runtime_error(std::format("Color attachment {} for framebuffer id: {} is not loaded", i, m_Id));

			Texture2D copy = *color;
			m_Color2.push_back(AssetManager::Put(std::move(copy)));
		}

		if (HasDepth())
		{
			auto depth = AssetManager::GetAsset(m_Depth).Asset;
			if (!depth)
				throw std::runtime_error(std::format("Depth for framebuffer id: {} is not loaded", m_Id));

			Texture2D copy = *depth;
			m_Depth2 = AssetManager::Put(std::move(copy));
		}
	}

	void Framebuffer::Swap()
	{
		std::swap(m_Color, m_Color2);
		std::swap(m_Depth, m_Depth2);

		Bind();

		u32 i = 0;
		for (const auto& colorA : m_Color)
		{
			auto color = AssetManager::GetAsset(colorA).Asset;

			if (!color)
				throw std::runtime_error(std::format("Color attachment {} for framebuffer id: {} is not loaded", i, m_Id));

			GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, color->Id(), 0));

			i++;
		}

		if (HasDepth())
		{
			auto [depth, g2] = AssetManager::GetAsset(m_Depth);
			if (!depth)
				throw std::runtime_error(std::format("Depth for framebuffer id: {} is not loaded", m_Id));

			GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth->Id(), 0));
		}
	}

	void Framebuffer::Resize(u32 w, u32 h)
	{
		Bind();

		u32 i = 0;
		for (const auto& colorA : m_Color)
		{
			auto color = AssetManager::GetAsset(colorA).Asset;

			if (!color)
				throw std::runtime_error(std::format("Color attachment {} for framebuffer id: {} is not loaded", i, m_Id));

			color->Properties().w = w;
			color->Properties().h = h;
			color->Update();

			GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, color->Properties().Type, color->Id(), 0));
			i++;
		}

		i = 0;
		for (const auto& colorA : m_Color2)
		{
			auto color = AssetManager::GetAsset(colorA).Asset;

			if (!color)
				throw std::runtime_error(std::format("Swap color attachment {} for framebuffer id: {} is not loaded", i, m_Id));

			i++;

			color->Properties().w = w;
			color->Properties().h = h;
			color->Update();
		}

		if (HasDepth())
		{
			auto [depth, g2] = AssetManager::GetAsset(m_Depth);
			auto [depth2, g4] = AssetManager::GetAsset(m_Depth2);

			if (!depth)
				throw std::runtime_error(std::format("Depth for framebuffer id: {} is not loaded", m_Id));

			depth->Properties().w = w;
			depth->Properties().h = h;
			depth->Update();
			GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth->Properties().Type, depth->Id(), 0));

			if (!depth2 && m_Depth2.Id != 0)
				throw std::runtime_error(std::format("Swap depth for framebuffer id: {} is not loaded", m_Id));

			if (depth2)
			{
				depth2->Properties().w = w;
				depth2->Properties().h = h;
				depth2->Update();
			}
		}
		
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

