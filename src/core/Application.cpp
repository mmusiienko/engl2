#include "core/Application.h"

#include <stdexcept>
#include <vector>

#include "InputHandler.h"

#include "Global.h"
#include "math/Math.h"

#include "renderer/base/Material.h"
#include "resources/importers/AssetManager.h"

#include "ecs/entity.h"
#include "ecs/components/Components.h"
#include "ecs/systems/Systems.h"
#include "ecs/systems/WaterSystem.h"
#include "ecs/systems/CloudSystem.h"
#include "ecs/systems/TerrainSystem.h"

#include "ui/Ui.h"


namespace EnGl 
{
	u32 MAX_LIGHTS = 32;

	void Application::CreateFramebuffer(u32 w, u32 h)
	{
		auto color = AssetManager::Put<Texture2D>(
			Texture2D{ 
				w, h, 
				Texture::CreationInfoFromData{
					.CpuFormat = GL_RGBA,
					.GpuFormat = GL_RGBA32F,
					.DataType = GL_UNSIGNED_BYTE,
				}
			}
		);
		auto depth = AssetManager::Put<Texture2D>(
			Texture2D{ 
				w, h,
				Texture::CreationInfoFromData{
					.CpuFormat = GL_DEPTH_COMPONENT,
					.GpuFormat = GL_DEPTH_COMPONENT32F,
					.DataType = GL_FLOAT
				}
			}
		);

		Framebuffer::CreationInfo info;
		info.ColorAttachments = { color };
		info.DepthAttachment = depth;
		m_Framebuffer = make_scope<Framebuffer>(std::move(info));
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
		glPatchParameteri(GL_PATCH_VERTICES, 4);
		glfwWindowHint(GLFW_SAMPLES, 4);
		glEnable(GL_MULTISAMPLE);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_GREATER);
		glClearDepth(0.0f);
		glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
		glDepthRange(0.0, 1.0);

		glfwSwapInterval(0);

		glfwSetWindowUserPointer(m_Window, this);

		Entity camera = World().eEntity().Create<
			Component::Transform,
			Component::Velocity,
			Component::ViewMatrix,
			Component::ProjectionMatrix,
			Component::PerspectiveProjection
		>([](Component::Transform& transform, Component::Velocity& mov, auto&, auto&, Component::PerspectiveProjection& cam)
		{
			mov.Speed = 10.0f;
			transform.Position = { 0.0f, 0.0f, 0.0f };

			cam.Aspect = 16.0f / 9.0f;
			cam.FovDegree = 45.0f;
			cam.NearPlane = 0.1f;
			cam.FarPlane = 1000.0f;
		}, "Camera1");

		auto colorTexHandle = m_Framebuffer->Color()[0];
		auto depthTexHandle = m_Framebuffer->Depth();

		auto mat = make_scope<Material::MainQuad>();
		mat->TextureHandle = colorTexHandle;

		auto mainQuadMat = AssetManager::PutScope<Material::Base>(std::move(mat));
		auto unlitMat = AssetManager::PutScope<Material::Base>(make_scope<Material::Unlit>(glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f }));
		auto worldPlaneMat = AssetManager::PutScope<Material::Base>(make_scope<Material::Unlit>(glm::vec4{ 0.0f, 0.7f, 0.0f, 1.0f }));
		auto coordinatePlaneMat = AssetManager::PutScope<Material::Base>(make_scope<Material::CoordinatePlane>());
		auto unlitIMat = AssetManager::PutScope<Material::Base>(make_scope<Material::Unlit>(true));

		auto quad = World().eEntity().Create<
			Component::Transform, Component::ModelMatrix, Component::RenderedModel
		>([mainQuadMat](auto&, auto&, Component::RenderedModel& model) -> void
		{
			model.Model = StaticModel::Quad(mainQuadMat);
			model.Layer = Component::RenderLayer::SCREEN_QUAD;
		}, "ScreenQuad");

//		auto worldPlane = World().eEntity().Create<
//			Component::Transform, Component::ModelMatrix, Component::RenderedModel
//		>([worldPlaneMat](Component::Transform& transform, auto&, Component::RenderedModel& model) -> void
//		{
//			transform.Scale = glm::vec3{ 1000.0f };
//			transform.Rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
//			model.Model = StaticModel::Quad(worldPlaneMat);
//		});
//
//		auto coordinatePlane = World().eEntity().Create<
//			Component::Transform, Component::ModelMatrix, Component::RenderedModel
//		>([coordinatePlaneMat](Component::Transform& transform, auto&, Component::RenderedModel& model) -> void
//		{
//			transform.Scale = glm::vec3{ 2000.0f };
//			transform.Rotation = glm::angleAxis(glm::radians(90.0f), glm::vec3{ 1.0f, 0.0f, 0.0f });
//			model.Model = StaticModel::Quad(coordinatePlaneMat);
//			model.Layer = Component::RenderLayer::OL;
//		});


		AssetImporter<Model>::Params towerParams{ AssetManager::MODEL_DIR / "floating-castle" / "source" / "Mineways2Skfb.obj"};
		auto towerHandle = AssetManager::Load<Model>(towerParams);

		AssetImporter<Cubemap>::Params cparams
		{
		{
			AssetManager::TEXTURE_DIR / "skybox" / "bluesky" / "r.png",
			AssetManager::TEXTURE_DIR / "skybox" / "bluesky" / "l.png",
			AssetManager::TEXTURE_DIR / "skybox" / "bluesky" / "t.png",
			AssetManager::TEXTURE_DIR / "skybox" / "bluesky" / "d.png",
			AssetManager::TEXTURE_DIR / "skybox" / "bluesky" / "b.png",
			AssetManager::TEXTURE_DIR / "skybox" / "bluesky" / "f.png"
		}, {false, false, false, true, false, false}
		};

		auto cubemapHadle = AssetManager::Load<Cubemap>(cparams);
		auto cubemapMat = AssetManager::PutScope<Material::Base>(make_scope<Material::CubemapObj>(cubemapHadle));
		auto cubemapModelHandle = StaticModel::Cube(cubemapMat);

		Ui ui{ m_Window };

		auto cubemapEntity = World().eEntity().Create<Component::Transform, Component::ModelMatrix, Component::RenderedModel>(
			[=](Component::Transform& transform, auto&, Component::RenderedModel& model, auto&...) -> void
			{
				model.Model = cubemapModelHandle;
				model.Layer = Component::RenderLayer::CUBEMAP;
				model.MeshIdx = 0;
			}, "Cubemap"
		);

		World().eEntity().Create<Component::Transform, Component::ModelMatrix, Component::RenderedModel>(
			[=](Component::Transform& transform, auto&, Component::RenderedModel& model, auto&...) -> void
			{
				transform.Position = { 0.0f, 0.0f, 0.0f };
				model.Model = towerHandle;
				model.MeshIdx = -1;
			}, "Tower"
		);

		World().eEntity().Create<Component::Transform, Component::ModelMatrix, Component::RenderedModel>(
			[=](Component::Transform& transform, auto&, Component::RenderedModel& model, auto&...) -> void
			{
				transform.Position = { 0.0f, 50.0f, 0.0f };
				model.Model = StaticModel::Quad(coordinatePlaneMat);
				model.Layer = Component::RenderLayer::OL;
				model.MeshIdx = 0;
			}, "CoordinatePlane"
		);

		auto dirLight = World().eEntity().Create<Component::Transform, Component::DirectionalLight>(
			[=](Component::Transform& transform, Component::DirectionalLight& l) -> void
			{
				transform.Rotation = glm::angleAxis(glm::radians(45.0f), Constants::FORWARD) * glm::angleAxis(glm::radians(-90.0f), Constants::UP);
				l.Color = glm::vec3{253.0f / 255.0f, 183.0f / 255.0f, 72.0f / 255.0f};
			}, "dirLight"
		);

		World().eEntity().Create<Component::Transform, Component::PointLight>(
			[=](Component::Transform& transform, Component::PointLight& l) -> void
			{
				transform.Position = glm::vec3{0.0f, 110.0f, 0.0f};
				l.Color = glm::vec3{ 1.0f };
				l.Intensity = 1.0f;
			}, "pointLight"
		);

		GameContext context{};
		f64 timeLastFrame = 0.0f;
		
		context.Cubemap.Asset = cubemapHadle;
		context.Cubemap.Id = cubemapEntity;
		auto [depth, g1] = m_Framebuffer->Depth();
		context.Framebuffer.DepthTextureOpaque = AssetManager::Put(Texture2D{ depth });

		Framebuffer::CreationInfo dinfo0;
		dinfo0.DepthAttachment = AssetManager::Put<Texture2D>(
			Texture2D{
				context.Renderer.CascadedShadowMap.TextureSize[0], context.Renderer.CascadedShadowMap.TextureSize[0],
				Texture::CreationInfoFromData{
					.CpuFormat = GL_DEPTH_COMPONENT,
					.GpuFormat = GL_DEPTH_COMPONENT24,
					.DataType = GL_FLOAT,
					.Common = {.Wrap = GL_CLAMP_TO_BORDER, .MinFilter = GL_NEAREST, .MagFilter = GL_NEAREST, .BorderColor = glm::vec4{1.0f}}
				}
			}
		);
		Framebuffer dirShadowFramebuffer0{dinfo0};
		context.Renderer.CascadedShadowMap.DirShadowFramebuffer[0] = &dirShadowFramebuffer0;

		Framebuffer::CreationInfo dinfo1;
		dinfo1.DepthAttachment = AssetManager::Put<Texture2D>(
			Texture2D{
				context.Renderer.CascadedShadowMap.TextureSize[1], context.Renderer.CascadedShadowMap.TextureSize[1],
				Texture::CreationInfoFromData{
					.CpuFormat = GL_DEPTH_COMPONENT,
					.GpuFormat = GL_DEPTH_COMPONENT24,
					.DataType = GL_FLOAT,
					.Common = {.Wrap = GL_CLAMP_TO_BORDER, .MinFilter = GL_NEAREST, .MagFilter = GL_NEAREST, .BorderColor = glm::vec4{1.0f}}
				}
			}
		);
		Framebuffer dirShadowFramebuffer1{ dinfo1 };
		context.Renderer.CascadedShadowMap.DirShadowFramebuffer[1] = &dirShadowFramebuffer1;

		Framebuffer::CreationInfo dinfo2;
		dinfo2.DepthAttachment = AssetManager::Put<Texture2D>(
			Texture2D{
				context.Renderer.CascadedShadowMap.TextureSize[2], context.Renderer.CascadedShadowMap.TextureSize[2],
				Texture::CreationInfoFromData{
					.CpuFormat = GL_DEPTH_COMPONENT,
					.GpuFormat = GL_DEPTH_COMPONENT24,
					.DataType = GL_FLOAT,
					.Common = {.Wrap = GL_CLAMP_TO_BORDER, .MinFilter = GL_NEAREST, .MagFilter = GL_NEAREST, .BorderColor = glm::vec4{1.0f}}
				}
			}
		);
		Framebuffer dirShadowFramebuffer2{ dinfo2 };
		context.Renderer.CascadedShadowMap.DirShadowFramebuffer[2] = &dirShadowFramebuffer2;


		Entity camera0 = World().eEntity().Create<
			Component::Transform,
			Component::Velocity,
			Component::ViewMatrix,
			Component::ProjectionMatrix,
			Component::OrthogonalProjection
		>([](Component::Transform& transform, Component::Velocity& mov, auto&, auto&, Component::OrthogonalProjection& cam)
			{
				cam.NearPlane = 0.1f;
				cam.FarPlane = 5000.0f;

				cam.Bottom = -1000.0f;
				cam.Right = 1000.0f;
				cam.Left = -1000.0f;
				cam.Top = 1000.0f;

				mov.Speed = 100.0f;
				transform.Position = { 0.0f, 100.0f, 0.0f };
				transform.Rotation =
					glm::angleAxis(glm::radians(-35.264f), glm::vec3{ 1.0f, 0.0f, 0.0f });
			}, "Camera0");

		Entity camera1 = World().eEntity().Create<
			Component::Transform,
			Component::Velocity,
			Component::ViewMatrix,
			Component::ProjectionMatrix,
			Component::OrthogonalProjection
		>([](Component::Transform& transform, Component::Velocity& mov, auto&, auto&, Component::OrthogonalProjection& cam)
			{
				cam.NearPlane = 0.1f;
				cam.FarPlane = 5000.0f;

				cam.Bottom = -1000.0f;
				cam.Right = 1000.0f;
				cam.Left = -1000.0f;
				cam.Top = 1000.0f;

				mov.Speed = 100.0f;
				transform.Position = { 0.0f, 100.0f, 0.0f };
				transform.Rotation =
					glm::angleAxis(glm::radians(-35.264f), glm::vec3{ 1.0f, 0.0f, 0.0f });
			}, "Camera1");


		Entity camera2 = World().eEntity().Create<
			Component::Transform,
			Component::Velocity,
			Component::ViewMatrix,
			Component::ProjectionMatrix,
			Component::OrthogonalProjection
		>([](Component::Transform& transform, Component::Velocity& mov, auto&, auto&, Component::OrthogonalProjection& cam)
			{
				cam.NearPlane = 0.1f;
				cam.FarPlane = 5000.0f;

				cam.Bottom = -1000.0f;
				cam.Right = 1000.0f;
				cam.Left = -1000.0f;
				cam.Top = 1000.0f;

				mov.Speed = 100.0f;
				transform.Position = { 0.0f, 100.0f, 0.0f };
				transform.Rotation =
					glm::angleAxis(glm::radians(-35.264f), glm::vec3{ 1.0f, 0.0f, 0.0f });
			}, "Camera2");

		context.Renderer.CascadedShadowMap.CascadeCamera[0] = camera0;
		context.Renderer.CascadedShadowMap.CascadeCamera[1] = camera1;
		context.Renderer.CascadedShadowMap.CascadeCamera[2] = camera2;

		context.Framebuffer.MainFramebuffer = m_Framebuffer.get();
		context.DirLight.Id = dirLight;

		context.Camera.Cameras.push_back({ .Entity = camera, .CanRotate = true });
		context.Camera.Cameras.push_back({ .Entity = camera0 });
		context.Camera.Cameras.push_back({ .Entity = camera1 });
		context.Camera.Cameras.push_back({ .Entity = camera2 });


		World().eSystem<GameContext>()
			.Add<System::Input>()
			.Add<System::Movement>()
			.Add<System::ConstantRotation>()
			.Add<System::FollowSnap>()
			.Add<System::ModelMatrix>()
			.Add<System::ViewMatrix>()
			.Add<System::ProjectionMatrix>()
			.Add<System::UpdateCameras>()
			.Add<System::CollisionDetection>()
			.Add<System::Gravity>()
			.Add<System::CollectLights>()
			.Add<System::CascadedShadowMaps>()
			.Add<WaterSystem>()
			.Add<System::PrepareDebugDraw>()
			.Add<System::BundleDirtyMaterials>()
			.Add<System::RenderToFramebuffer>()
			.Add<TerrainSystem>()
			.Add<CloudSystem>()
			.Add<System::UpdateCubemap>()
			.Add<System::RenderToDefaultFramebuffer>()
			.Add<System::Cleanup>()
			.Init();

		while (!glfwWindowShouldClose(m_Window))
		{
			glfwPollEvents();

			auto [d, g] = AssetManager::GetAsset(m_Framebuffer->Depth());
			InputHandler::ProcessInputEvents(m_Window);
				
			f64 time = glfwGetTime();
			f32 deltaTime = static_cast<f32>(time - timeLastFrame);
			timeLastFrame = time;
			context.Time = time;
			context.DeltaTime = deltaTime;

				if (!Global::IsPaused)
				{
					World().eSystem<GameContext>().Run(context);
				}
				ui.Render(context, World().eEntity(), World().eSystem<GameContext>());
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
		glfwSetScrollCallback(m_Window, InputHandler::ScrollCallback);

		CreateFramebuffer(static_cast<i32>(Global::WindowResolution.x), static_cast<i32>(Global::WindowResolution.y));

		spdlog::info("Initializing window complete");

		Global::StartUp();
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