
#include <stdexcept>
#include <vector>
#include <iostream>

#include "Application.h"
#include "InputHandler.h"

#include "Global.h"
#include "../math/Math.h"

#include "../renderer/base/Shader.h"
#include "../renderer/base/Material.h"
#include "../resources/importers/AssetManager.h"

#include "../ecs/entity.h"
#include "../ecs/components/Components.h"
#include "../ecs/systems/Systems.h"
#include "../ecs/systems/WaterSystem.h"
#include "../ecs/systems/CloudSystem.h"
#include "../ecs/systems/FractalSystem.h"

#include "../ui/Ui.h"


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
					.GpuFormat = GL_DEPTH_COMPONENT24,
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

		glfwSwapInterval(0);

		glfwSetWindowUserPointer(m_Window, this);

		EcsImpl::Entity camera = World().eEntity().Create<
			Component::Transform,
			Component::Velocity,
			Component::ModelMatrix,
			Component::ViewMatrix,
			Component::ProjectionMatrix,
			Component::PerspectiveProjection
		>([](Component::Transform& transform, Component::Velocity& mov, auto&, auto&, auto&, Component::PerspectiveProjection& cam)
		{
			mov.Speed = 100.0f;
			transform.Position = { 0.0f, 0.0f, 0.0f };
			transform.Rotation =
				glm::angleAxis(glm::radians(-35.264f), glm::vec3{ 1.0f, 0.0f, 0.0f });

			cam.Aspect = 16.0f / 9.0f;
			cam.FovDegree = 45.0f;
			cam.NearPlane = 0.1f;
			cam.FarPlane = 1000.0f;
		}, "Camera1");

		EcsImpl::Entity camera2 = World().eEntity().Create<
			Component::Transform,
			Component::Velocity,
			Component::ModelMatrix,
			Component::ViewMatrix,
			Component::ProjectionMatrix,
			Component::OrthogonalProjection
		>([](Component::Transform& transform, Component::Velocity& mov, auto&, auto&, auto&, Component::OrthogonalProjection& cam)
		{
			cam.NearPlane = 0.1f;
			cam.FarPlane = 1000.0f;

			cam.Bottom = -200.0f;
			cam.Right = 200.0f;
			cam.Left = -200.0f;
			cam.Top = 200.0f;

			mov.Speed = 100.0f;
			transform.Position = { 0.0f, 100.0f, 0.0f };
			transform.Rotation =
				glm::angleAxis(glm::radians(-35.264f), glm::vec3{ 1.0f, 0.0f, 0.0f });
		}, "Camera2");

		auto shaderHandle = AssetManager::Load<Shader>(AssetManager::GRAPHICS_SHADER_DIR / "ScreenSpaceTextured");
		auto colorTexHandle = m_Framebuffer->Color()[0];
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

		auto cubeModelIHandle = StaticModel::CubeInstanced(unlitIMat);
		auto cubeModelHandle = StaticModel::Cube(unlitMat);
//		AssetImporter<Model>::Params params{ AssetManager::MODEL_DIR / "buildings" / "model" / "All.fbx", true };
//		auto buildingHandle = AssetManager::Load<Model>(params);

		//AssetImporter<Model>::Params shipparams{ AssetManager::MODEL_DIR / "ship" / "ss-norrtelje-lowpoly" / "source" / "norrtelje" / "norrtelje.fbx" };
//		AssetImporter<Model>::Params shipparams{ AssetManager::MODEL_DIR / "ship" / "submarine.fbx"};
//		auto shipHandle = AssetManager::Load<Model>(shipparams);
//
		AssetImporter<Model>::Params gunParams{ AssetManager::MODEL_DIR / "Cerberus_by_Andrew_Maximov" / "Cerberus_LP.FBX"};
//		auto gunHandle = AssetManager::Load<Model>(gunParams);
//
//		auto gun = AssetManager::GetAsset(gunHandle).Asset;
//		if (gun)
//		{
//			spdlog::info(gun->TotalMeshes());
//
//			scope<Material::PBRTextured> mat = make_scope<Material::PBRTextured>();
//			mat->AlbedoHandle = AssetManager::Load<Texture2D>(AssetManager::MODEL_DIR / "Cerberus_by_Andrew_Maximov" / "textures" / "Cerberus_A.tga");
//			mat->MetallicHandle = AssetManager::Load<Texture2D>(AssetManager::MODEL_DIR / "Cerberus_by_Andrew_Maximov" / "textures" / "Cerberus_M.tga");
//			mat->RoughnessHandle = AssetManager::Load<Texture2D>(AssetManager::MODEL_DIR / "Cerberus_by_Andrew_Maximov" / "textures" / "Cerberus_R.tga");
//			mat->AOHandle = AssetManager::Load<Texture2D>(AssetManager::MODEL_DIR / "Cerberus_by_Andrew_Maximov" / "textures" / "Raw" / "Cerberus_AO.tga");
//			gun->GetSubmesh(0).Material = AssetManager::Put<scope<Material::Base>>(std::move(mat));
//		}

		AssetImporter<Cubemap>::Params cparams{ AssetManager::TEXTURE_DIR / "skybox" / "2.png" };

//		AssetImporter<Cubemap>::Params cparams
//		{
//		{
//			AssetManager::TEXTURE_DIR / "skybox" / "cosmos" / "right.png",
//			AssetManager::TEXTURE_DIR / "skybox" / "cosmos" / "left.png",
//			AssetManager::TEXTURE_DIR / "skybox" / "cosmos" / "top.png",
//			AssetManager::TEXTURE_DIR / "skybox" / "cosmos" / "bottom.png",
//			AssetManager::TEXTURE_DIR / "skybox" / "cosmos" / "front.png",
//			AssetManager::TEXTURE_DIR / "skybox" / "cosmos" / "back.png"
//		}
//		};

		auto cubemapHadle = AssetManager::Load<Cubemap>(cparams);
		auto cubemapMat = AssetManager::Put<scope<Material::Base>>(make_scope<Material::CubemapObj>(cubemapHadle));
		auto cubemapModelHandle = StaticModel::Cube(cubemapMat);

		Ui ui{ m_Window };

		World().eEntity().Create<Component::Transform, Component::ModelMatrix, Component::RenderedModel>(
			[=](Component::Transform& transform, auto&, Component::RenderedModel& model, auto&...) -> void
			{
				model.Model = cubemapModelHandle;
				model.Layer = Component::RenderLayer::CUBEMAP;
				model.MeshIdx = 0;
			}, "Cubemap"
		);

//		World().eEntity().Create<Component::Transform, Component::Movement, Component::ModelMatrix, Component::RenderedModel>(
//			[=](Component::Transform& transform, Component::Movement& mov, auto&, Component::RenderedModel& model, auto&...) -> void
//			{
//				mov.SetDirection({ 1.0f, 0.0, 1.0f });
//				mov.Velocity = 10.0f;
//				transform.Position = { 0.0f, 0.0f, 0.0f };
//				model.Model = shipHandle;
//				model.MeshIdx = -1;
//			}, "Ship"
//		);

		World().eEntity().Create<Component::Transform, Component::ModelMatrix, Component::RenderedModel>(
			[=](Component::Transform& transform, auto&, Component::RenderedModel& model, auto&...) -> void
			{
				transform.Position = { 0.0f, 50.0f, 0.0f };
				model.Model = StaticModel::Quad(coordinatePlaneMat);
				model.Layer = Component::RenderLayer::OL;
				model.MeshIdx = 0;
			}, "CoordinatePlane"
		);
		scope<Material::PBR> pbrmat = make_scope<Material::PBR>();
		pbrmat->Albedo = { 0.8f, 0.0f, 0.0f };
		pbrmat->Metallic = 0.0f;
		pbrmat->Roughness = 0.4f;
		pbrmat->AO = 0.1f;
	
//		scope<Material::PBRTextured> pbrmat = make_scope<Material::PBRTextured>();
//		pbrmat->AlbedoHandle = AssetManager::Load<Texture2D>(AssetManager::TEXTURE_DIR / "pbr" / "rustediron1-alt2-bl" / "rustediron2_basecolor.png");
//		pbrmat->MetallicHandle = AssetManager::Load<Texture2D>(AssetManager::TEXTURE_DIR / "pbr" / "rustediron1-alt2-bl" / "rustediron2_metallic.png");
//		pbrmat->RoughnessHandle = AssetManager::Load<Texture2D>(AssetManager::TEXTURE_DIR / "pbr" / "rustediron1-alt2-bl" / "rustediron2_roughness.png");
		//mat->AOHandle = AssetManager::Load<Texture2D>(AssetManager::MODEL_DIR / "Cerberus_by_Andrew_Maximov" / "textures" / "Raw" / "Cerberus_AO.tga");
		auto pbrmathandle = AssetManager::Put<scope<Material::Base>>(std::move(pbrmat));
		World().eEntity().Create<Component::Transform, Component::ModelMatrix, Component::RenderedModel>(
			[=](Component::Transform& transform, auto&, Component::RenderedModel& model, auto&...) -> void
			{
				transform.Position = { 0.0f, 100.0f, 0.0f };
				transform.Scale = glm::vec3{10.0f};

				model.Model = StaticModel::Sphere(pbrmathandle);
			}, "cube"
		);

		World().eEntity().Create<Component::Transform, Component::ModelMatrix, Component::RenderedModel>(
			[=](Component::Transform& transform, auto&, Component::RenderedModel& model, auto&...) -> void
			{
				transform.Position = { 30.0f, 100.0f, 0.0f };
				transform.Scale = glm::vec3{ 10.0f };

				model.Model = StaticModel::Sphere(pbrmathandle);
			}, "cube"
		);

		World().eEntity().Create<Component::Transform, Component::ModelMatrix, Component::RenderedModel>(
			[=](Component::Transform& transform, auto&, Component::RenderedModel& model, auto&...) -> void
			{
				transform.Scale = glm::vec3{ 1000.0f };
				transform.Rotation = glm::angleAxis(glm::radians(-90.0f), Constants::RIGHT);

				model.Model = StaticModel::Quad(pbrmathandle);
			}, "plane"
		);

		World().eEntity().Create<Component::Transform, Component::DirectionalLight>(
			[=](Component::Transform& transform, Component::DirectionalLight& l) -> void
			{
				transform.Rotation = glm::angleAxis(glm::radians(45.0f), Constants::FORWARD) * glm::angleAxis(glm::radians(-90.0f), Constants::UP);
				l.Color = glm::vec3{1.0f};
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

		World().eSystem<GameContext>()
			.Add<System::Input>()
			.Add<System::Movement>()
			.Add<System::ModelMatrix>()
			.Add<System::ViewMatrix>()
			.Add<System::ProjectionMatrix>()
			.Add<System::UpdateCameras>()
			.Add<System::CollectLights>()
			//.Add<WaterSystem>()
			.Add<System::PrepareDebugDraw>()
			.Add<System::BundleDirtyMaterials>()
			.Add<System::RenderToFramebuffer>()
			//.Add<CloudSystem>()
			.Add<System::RenderToDefaultFramebuffer>()
			.Add<System::Cleanup>()
			.Init();

		GameContext context{};
		f64 timeLastFrame = 0.0f;
		context.Camera.Cameras.push_back({ .Entity = camera, .CanRotate = true });
		context.Camera.Cameras.push_back({ .Entity = camera2 });
		auto [cubemap, g0] = AssetManager::GetAsset(cubemapHadle);
		context.Cubemap = cubemap;
		auto [depth, g1] = m_Framebuffer->Depth();
		context.Framebuffer.DepthTextureOpaque = AssetManager::Put(Texture2D{ depth });
		m_Framebuffer->DoubleBuffer();
		Framebuffer::CreationInfo dinfo;
		dinfo.DepthAttachment = AssetManager::Put<Texture2D>(
			Texture2D{
				1024, 1024,
				Texture::CreationInfoFromData{
					.CpuFormat = GL_DEPTH_COMPONENT,
					.GpuFormat = GL_DEPTH_COMPONENT24,
					.DataType = GL_FLOAT,
					.Common = {.Wrap = GL_CLAMP_TO_BORDER, .MinFilter = GL_NEAREST, .MagFilter = GL_NEAREST, .BorderColor = glm::vec4{1.0f}}
				}
			}
		);

		Framebuffer dirShadowFramebuffer{dinfo};
		context.Camera.DirShadowCameraIdx = 1;

		context.Framebuffer.MainFramebuffer = m_Framebuffer.get();
		context.Framebuffer.DirShadowFramebuffer = &dirShadowFramebuffer;

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
			m_Framebuffer->Swap();

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