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

		void Bind() const;
		void Resize(u32 w, u32 h);

		static void Unbind();

		inline AssetHandle<Texture2D> Color() const { return m_Color; }
		inline AssetHandle<Texture2D> Depth() const { return m_Depth; }
	private:
		AssetHandle<Texture2D> m_Depth;
		AssetHandle<Texture2D> m_Color;
	};
}