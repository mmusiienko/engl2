#pragma once
#include "./Systems.h"


namespace EnGl
{
	class JuliaFractalSystem : public SystemImpl
	{
	public:
		void Init(EcsImpl::EntityManager& manager) override;

	private:
		EcsImpl::Entity m_ScreenQuad;
		AssetHandle<Shader> m_Shader = AssetManager::Load<Shader>(AssetManager::GRAPHICS_SHADER_DIR / "JuliaFractal");
	};

	class Fractal2DSystem : public SystemImpl
	{
	public:
		void Init(EcsImpl::EntityManager& manager) override;
		void Run(EcsImpl::EntityManager& manager, GameContext& context) override;

	private:
		f64 m_Zoom = 1.0f;
		glm::f64vec2 m_Position{};

		EcsImpl::Entity m_MandelbrotQuad = 0;
		EcsImpl::Entity m_JuliaQuad = 0;
		AssetHandle<Shader> m_Shader = AssetManager::Load<Shader>(AssetManager::GRAPHICS_SHADER_DIR / "JuliaFractal");
		friend class Fractal2DQuad;
	};
}
