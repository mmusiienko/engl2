

#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <iomanip>
#include <string>

#include "spdlog/spdlog.h"

#include "Application.h"
#include "InputHandler.h"

#include "Global.h"
#include "Constants.h"
#include "../math/Math.h"

#include "../renderer/base/Shader.h"
#include "../renderer/base/Texture.h"
#include "../renderer/base/Material.h"
#include "../renderer/base/Mesh.h"
#include "../resources/importers/AssetManager.h"

#include "../math/Random.h"

#include "../Printer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../ecs/entity.h"
#include "../ecs/components/Components.h"
#include "../ecs/systems/Systems.h"

#include "../ui/Ui.h"


namespace EnGl 
{
	u32 MAX_LIGHTS = 32;

	void Application::CreateFramebuffer(u32 w, u32 h)
	{
		m_Framebuffer = make_scope<Framebuffer>(w, h);
	}

	void Application::ResizeFramebuffer(u32 w, u32 h)
	{
		m_Framebuffer->Resize(w, h);
	}

	void Application::framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		if (width == 0 || height == 0)
			return;
		auto app = static_cast<Application*>(glfwGetWindowUserPointer(window));

		glViewport(0, 0, width, height);
		app->ResizeFramebuffer(static_cast<u32>(width), static_cast<u32>(height));
		Global::WindowResolution = { static_cast<u32>(width), static_cast<u32>(height) };
	}

	void Application::Run()
	{
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);

		glfwSwapInterval(0);

		glfwSetWindowUserPointer(m_Window, this);

		EcsImpl::Entity camera = World().eEntity().Create<
			Component::Transform,
			Component::Movement,
			Component::ModelMatrix,
			Component::ViewProjectionMatrix
		>([](Component::Transform& transform, Component::Movement& mov, auto&, auto&)
		{
			mov.Velocity = 100.0f;
			transform.Position = { 0.0f, 200.0f, 0.0f };
			transform.Rotation =
				glm::angleAxis(glm::radians(-35.264f), glm::vec3{ 1.0f, 0.0f, 0.0f });
		});

		EcsImpl::Entity camera2 = World().eEntity().Create<
			Component::Transform,
			Component::Movement,
			Component::ModelMatrix,
			Component::ViewProjectionMatrix
		>([](Component::Transform& transform, Component::Movement& mov, auto&, auto&)
			{
				mov.Velocity = 100.0f;
				transform.Position = { 0.0f, 200.0f, 0.0f };
				transform.Rotation =
					glm::angleAxis(glm::radians(-35.264f), glm::vec3{ 1.0f, 0.0f, 0.0f });
		});

		auto shaderHandle = AssetManager::Load<Shader>(AssetManager::GRAPHICS_SHADER_DIR / "ScreenSpaceTextured");
		auto colorTexHandle = m_Framebuffer->Color();
		auto depthTexHandle = m_Framebuffer->Depth();

		auto screenSpaceTexturedMat = AssetManager::Put<scope<Material::Base>>(make_scope<Material::ScreenSpaceTextured>(colorTexHandle));
		auto unlitMat = AssetManager::Put<scope<Material::Base>>(make_scope<Material::Unlit>(glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f }));
		auto worldPlaneMat = AssetManager::Put<scope<Material::Base>>(make_scope<Material::Unlit>(glm::vec4{ 0.0f, 0.7f, 0.0f, 1.0f }));
		auto coordinatePlaneMat = AssetManager::Put<scope<Material::Base>>(make_scope<Material::CoordinatePlane>());
		auto unlitIMat = AssetManager::Put<scope<Material::Base>>(make_scope<Material::Unlit>(true));

		auto quad = World().eEntity().Create<
			Component::Transform, Component::ModelMatrix, Component::RenderedModel
		>([screenSpaceTexturedMat](auto&, auto&, Component::RenderedModel& model) -> void
		{
			model.Model = StaticModel::Quad(screenSpaceTexturedMat);
			model.Layer = Component::RenderLayer::SCREEN_QUAD;
		});

		auto worldPlane = World().eEntity().Create<
			Component::Transform, Component::ModelMatrix, Component::RenderedModel
		>([worldPlaneMat](Component::Transform& transform, auto&, Component::RenderedModel& model) -> void
		{
			transform.Scale = glm::vec3{ 1000.0f };
			transform.Rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
			model.Model = StaticModel::Quad(worldPlaneMat);
		});

		auto coordinatePlane = World().eEntity().Create<
			Component::Transform, Component::ModelMatrix, Component::RenderedModel
		>([coordinatePlaneMat](Component::Transform& transform, auto&, Component::RenderedModel& model) -> void
		{
			transform.Scale = glm::vec3{ 2000.0f };
			transform.Rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
			model.Model = StaticModel::Quad(coordinatePlaneMat);
			model.Layer = Component::RenderLayer::OL;
		});

		auto cubeModelIHandle = StaticModel::CubeInstanced(unlitIMat);
		auto cubeModelHandle = StaticModel::Cube(unlitMat);
		AssetImporter<Model>::Params params{ AssetManager::MODEL_DIR / "buildings" / "model" / "All.fbx", true };
		auto buildingHandle = AssetManager::Load<Model>(params);

		Ui ui{ m_Window };

		size_t num = 1000;
		f32 spacing = 150.0f;
		for (size_t i = 0; i < num; i++)
		{
			for (size_t j = 0; j < num; j++)
			{
				World().eEntity().Create<Component::Transform, Component::ModelMatrix, Component::RenderedModel>(
					[=](Component::Transform& transform, auto&, Component::RenderedModel& model, auto&...) -> void
					{
						transform.Position = { spacing * i, 0.0f, spacing * j };
						model.Model = cubeModelIHandle;
						model.MeshIdx = 0;
					}
				);
			}
		}

		World().eSystem<GameContext>()
			.Add<System::Input>()
			.Add<System::Movement>()
			.Add<System::ModelMatrix>()
			.Add<System::ViewProjectionMatrix>()
			.Add<System::PrepareDebugDraw>()
			.Add<System::BundleDirtyMaterials>()
			.Add<System::RenderToFramebuffer>()
			.Add<System::BindDefaultFramebuffer>()
			.Add<System::RenderToDefaultFramebuffer>()
			.Add<System::Cleanup>();
			
		f64 timeLastFrame = 0.0f;
		GameContext context{};
		context.Camera.Cameras.push_back({ .Entity = camera });
		context.Camera.Cameras.push_back({ .Entity = camera2, .CanRotate = true });

		while (!glfwWindowShouldClose(m_Window))
		{
			glfwPollEvents();

			m_Framebuffer->Bind();

			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_DEPTH_TEST);
			
			f64 time = glfwGetTime();
			f32 deltaTime = static_cast<f32>(time - timeLastFrame);
			timeLastFrame = time;
			context.Time = time;
			context.DeltaTime = deltaTime;

			InputHandler::ProcessInputEvents(m_Window);
				
				if (!Global::IsPaused)
				{
					World().eSystem<GameContext>().Run(context);
				}

				ui.Render(context, World().eEntity());

			ui.Present();

			glfwSwapBuffers(m_Window);
			InputHandler::ResetState();
		}
	}

	Application::Application()
	{
		spdlog::info("Starting the application");

		InitGl();
		InitWindow();
	}

	void Application::InitGl()
	{
		static const u32 major = 4;
		static const u32 minor = 6;

		spdlog::info("Initializing glfw");
		spdlog::info("GL version: {}.{}", major, minor);

		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}

	void Application::InitWindow()
	{
		spdlog::info("Initializing window");

		Global::WindowResolution = { 1280, 720 };
		m_Window = glfwCreateWindow(static_cast<i32>(Global::WindowResolution.x), static_cast<i32>(Global::WindowResolution.y), "Window", NULL, NULL);

		if (m_Window == NULL)
		{
			glfwTerminate();
			throw std::runtime_error("Window could not be created");
		}
		glfwMakeContextCurrent(m_Window);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			throw std::runtime_error("Error getting gl functiona");
		}

		glViewport(0, 0, static_cast<i32>(Global::WindowResolution.x), static_cast<i32>(Global::WindowResolution.y));

		glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);

		glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		glfwSetCursorPosCallback(m_Window, InputHandler::CursorCallback);
		glfwSetKeyCallback(m_Window, InputHandler::KeyboardCallback);
		glfwSetMouseButtonCallback(m_Window, InputHandler::MouseCallback);

		CreateFramebuffer(static_cast<i32>(Global::WindowResolution.x), static_cast<i32>(Global::WindowResolution.y));

		spdlog::info("Initializing window complete");
	}

	Application::~Application()
	{
		spdlog::info("Terminating imgui");

		m_Framebuffer.reset();

		Global::ShutDown();

		spdlog::info("Terminating glfw");

		glfwTerminate();
	}
}