#pragma once

#include "ecs/systems/Systems.h"


namespace EnGl::System
{
	class JuliaFractalSystem : public SystemImpl
	{
	public:
		void Init(EcsImpl::EntityManager& manager) override;

	private:
		Entity m_ScreenQuad;
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

		Entity m_MandelbrotQuad = 0;
		Entity m_JuliaQuad = 0;
		AssetHandle<Shader> m_Shader = AssetManager::Load<Shader>(AssetManager::GRAPHICS_SHADER_DIR / "JuliaFractal");
		friend struct Fractal2DQuad;
	};
}
