#pragma once

#include "renderer/base/Texture.h"
#include "resources/importers/AssetManager.h"


namespace EnGl
{
	class Framebuffer : public Resource
	{
	public:
		struct CreationInfo
		{
			AssetHandle<Texture2D> DepthAttachment{};
			std::vector<AssetHandle<Texture2D>> ColorAttachments{};
		};

		Framebuffer(CreationInfo info);

		Framebuffer(Framebuffer&& other) noexcept
		{
			m_Depth = other.m_Depth;
			m_Depth2 = other.m_Depth2;
			m_Resolution = other.m_Resolution;
			m_Color = std::move(other.m_Color);
			m_Color2 = std::move(other.m_Color2);
			other.m_Depth = {};
			other.m_Depth2 = {};
			other.m_Color = {};
			other.m_Color2 = {};
		};

		Framebuffer& operator=(Framebuffer&& other) noexcept
		{
			std::swap(m_Id, other.m_Id);
			std::swap(m_Depth, other.m_Depth);
			std::swap(m_Depth2, other.m_Depth2);
			std::swap(m_Resolution, other.m_Resolution);
			std::swap(m_Color, other.m_Color);
			std::swap(m_Color2, other.m_Color2);
			return *this;
		};

		~Framebuffer();

		void DoubleBuffer();
		void Swap();
		void Bind() const;
		void Resize(u32 w, u32 h);

		static void Unbind();

		inline const std::vector<AssetHandle<Texture2D>>& Color() const { return m_Color; }
		inline AssetHandle<Texture2D> Depth() const { return m_Depth; }
		inline const std::vector<AssetHandle<Texture2D>>& ColorLastFrame() const { return m_Color2; }
		inline AssetHandle<Texture2D> DepthLastFrame() const { return m_Depth2; }
		inline const glm::uvec2& Resolution() const { return m_Resolution; }

		inline bool HasDepth() const { return m_Depth.Id != 0; }
	private:
		AssetHandle<Texture2D> m_Depth{};
		std::vector<AssetHandle<Texture2D>> m_Color{};
		
		AssetHandle<Texture2D> m_Depth2{};
		std::vector<AssetHandle<Texture2D>> m_Color2{};

		glm::uvec2 m_Resolution{ 0u };
	};
}