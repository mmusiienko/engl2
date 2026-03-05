#pragma once
#include "Texture.h"
#include "../../resources/importers/AssetManager.h"


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
		inline const glm::vec2& Resolution() const { return m_Resolution; }

		inline bool HasDepth() const { return m_Depth.Id != 0; }
	private:
		AssetHandle<Texture2D> m_Depth{};
		std::vector<AssetHandle<Texture2D>> m_Color{};
		
		AssetHandle<Texture2D> m_Depth2{};
		std::vector<AssetHandle<Texture2D>> m_Color2{};

		glm::vec2 m_Resolution{ 1.0f };
	};
}