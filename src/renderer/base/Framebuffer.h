#pragma once
#include "Texture.h"
#include "../../resources/importers/AssetManager.h"


namespace EnGl
{
	class Framebuffer : public Resource
	{
	public:
		Framebuffer(u32 w, u32 h);

		~Framebuffer();

		void Swap();
		void Bind() const;
		void Resize(u32 w, u32 h);

		static void Unbind();

		inline AssetHandle<Texture2D> Color() const { return m_Color; }
		inline AssetHandle<Texture2D> Depth() const { return m_Depth; }
		inline AssetHandle<Texture2D> ColorLastFrame() const { return m_Color; }
		inline AssetHandle<Texture2D> DepthLastFrame() const { return m_Depth; }
		inline const glm::vec2& Resolution() const { return m_Resolution; }
	private:
		AssetHandle<Texture2D> m_Depth;
		AssetHandle<Texture2D> m_Color;
		
		AssetHandle<Texture2D> m_Depth2;
		AssetHandle<Texture2D> m_Color2;

		glm::vec2 m_Resolution{ 1.0f };
	};
}