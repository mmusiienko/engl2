#include "ecs/systems/FractalSystem.h"

#include "resources/StaticModel.h"


namespace EnGl::System
{
	struct JuliaFractalQuad : public Material::Base
	{
		JuliaFractalQuad() : Base(AssetManager::GRAPHICS_SHADER_DIR / "JuliaFractal") {}

		void SetCommonUniforms(Shader* shader, const GameContext& context) override
		{
			Base::SetCommonUniforms(shader, context);

			auto cam = context.Camera.Get();
			shader->SetUniform("uInvProjection", cam.InverseProjection);
			shader->SetUniform("uInvView", cam.InverseView);
			shader->SetUniform("uCameraPos", cam.Position);
			shader->SetUniform("uNear", cam.Near);
			shader->SetUniform("uFar", cam.Far);
			shader->SetUniform("uResolution", context.Framebuffer.MainFramebuffer->Resolution());
			shader->SetUniform("uTime", static_cast<f32>(context.Time));

			auto depth = AssetManager::GetAsset(context.Framebuffer.DepthTextureOpaque).Asset;
			if (depth)
			{
				shader->SetUniform("uDepth", *depth, 0);
			}
		}
	};

	void JuliaFractalSystem::Init(EcsImpl::EntityManager& manager)
	{
		m_ScreenQuad = manager.Create<Component::Transform, Component::ModelMatrix, Component::RenderedModel>
		(
			[=](Component::Transform&, Component::ModelMatrix&, Component::RenderedModel& model)
			{
				auto mat = AssetManager::PutScope<Material::Base>(make_scope<JuliaFractalQuad>());

				model.Layer = Component::RenderLayer::SCREEN_SPACE;
				model.Model = StaticModel::Quad(mat);
			}
		);
	}

	struct Fractal2DQuad : public Material::Base
	{
		Fractal2DSystem& m_System;

		Fractal2DQuad(Fractal2DSystem& system, std::filesystem::path shaderPath) : m_System(system), Base(std::move(shaderPath)) {}

		void SetCommonUniforms(Shader* shader, const GameContext& context) override
		{
			Base::SetCommonUniforms(shader, context);

			auto cam = context.Camera.Get();
			shader->SetUniform("uCameraPosX", m_System.m_Position.x);
			shader->SetUniform("uCameraPosY", m_System.m_Position.y);
			shader->SetUniform("uResolution", context.Framebuffer.MainFramebuffer->Resolution());
			shader->SetUniform("uTime", static_cast<f32>(context.Time));
			shader->SetUniform("uZoom", m_System.m_Zoom);
		}
	};

	void Fractal2DSystem::Init(EcsImpl::EntityManager& manager)
	{
		m_MandelbrotQuad = manager.Create<Component::Transform, Component::ModelMatrix, Component::RenderedModel>
		(
			[=](Component::Transform& t, Component::ModelMatrix&, Component::RenderedModel& model)
			{
				auto mat = AssetManager::PutScope<Material::Base>(make_scope<Fractal2DQuad>(*this, AssetManager::GRAPHICS_SHADER_DIR / "MandelbrotFractal"));
				t.Scale = glm::vec3{ 0.5f, 1.0f, 1.0f };
				t.Position = glm::vec3{-0.5f, 0.0f, 0.0f};
				model.Layer = Component::RenderLayer::SCREEN_SPACE;
				model.Model = StaticModel::Quad(mat);
			}
		);

		m_JuliaQuad = manager.Create<Component::Transform, Component::ModelMatrix, Component::RenderedModel>
		(
			[=](Component::Transform& t, Component::ModelMatrix&, Component::RenderedModel& model)
			{
				auto mat = AssetManager::PutScope<Material::Base>(make_scope<Fractal2DQuad>(*this, AssetManager::GRAPHICS_SHADER_DIR / "JuliaFractal"));
				t.Scale = glm::vec3{ 0.5f, 1.0f, 1.0f };
				t.Position = glm::vec3{ 0.5f, 0.0f, 0.0f };
				model.Layer = Component::RenderLayer::SCREEN_SPACE;
				model.Model = StaticModel::Quad(mat);
			}
		);
	}

	void Fractal2DSystem::Run(EcsImpl::EntityManager& manager, GameContext& context)
	{
		glm::f64vec2 dir{};

		if (InputHandler::State.MouseBHeld[GLFW_MOUSE_BUTTON_LEFT])
		{
			m_Position += glm::f64vec2{ InputHandler::State.MouseDelta.x, -InputHandler::State.MouseDelta.y } * static_cast<f64>(context.DeltaTime)* m_Zoom;
		}
		else
		{
			if (InputHandler::State.KeysHeld[GLFW_KEY_W])
				dir.y += 1;
			if (InputHandler::State.KeysHeld[GLFW_KEY_S])
				dir.y -= 1;
			if (InputHandler::State.KeysHeld[GLFW_KEY_A])
				dir.x -= 1;
			if (InputHandler::State.KeysHeld[GLFW_KEY_D])
				dir.x += 1;

			if (dir.x != 0.0 || dir.y != 0.0)
			{
				m_Position += glm::normalize(dir) * static_cast<f64>(context.DeltaTime) * m_Zoom * 4.0;
			}
		}
		
		if (InputHandler::State.ScrollDelta != 0.0f)
		{
			m_Zoom *= exp(-InputHandler::State.ScrollDelta * 0.1);
		}

		if (InputHandler::State.KeysHeld[GLFW_KEY_C])
			m_Zoom = 1.0;
		if (InputHandler::State.KeysHeld[GLFW_KEY_V])
			m_Position = {};
		if (InputHandler::State.KeysHeld[GLFW_KEY_X])
			m_Zoom *= exp(-10.0 * static_cast<f64>(context.DeltaTime));
		if (InputHandler::State.KeysHeld[GLFW_KEY_Z])
			m_Zoom *= exp(10.0 * static_cast<f64>(context.DeltaTime));

		if (InputHandler::State.KeysPressed[GLFW_KEY_B])
			spdlog::info("Visualization center: {} + {}i", m_Position.x, m_Position.y);
	}
}