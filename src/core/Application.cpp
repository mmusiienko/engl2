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
#include "ecs/systems/renderer/RenderScreenQuad.h"
#include "ecs/systems/renderer/ResetRenderState.h"
#include "ecs/systems/renderer/Render.h"
#include "ecs/systems/renderer/Prepass.h"
#include "ecs/systems/renderer/DrawDebug.h"
#include "ecs/systems/renderer/techniques/CascadedShadowMap.h"
#include "ecs/systems/renderer/techniques/SSAO.h"
#include "ecs/systems/Animation.h"

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
					.GpuFormat = GL_RGBA16F,
					.DataType = GL_FLOAT,
				}
			}
		);

		auto normals = AssetManager::Put<Texture2D>(
			Texture2D{
				w, h,
				Texture::CreationInfoFromData{
					.CpuFormat = GL_RG,
					.GpuFormat = GL_RG16F,
					.DataType = GL_FLOAT,
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
		info.ColorAttachments = { color, normals };
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
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

		glfwSwapInterval(0);

		glfwSetWindowUserPointer(m_Window, this);

		Entity camera = World().eEntity().Create<
			Component::Transform,
			Component::Velocity,
			Component::ModelMatrix,
			Component::ViewMatrix,
			Component::ProjectionMatrix,
			Component::PerspectiveProjection
		>([](Component::Transform& transform, Component::Velocity& mov, auto&, auto&, auto&, Component::PerspectiveProjection& cam)
		{
			mov.Speed = 10.0f;
			transform.Position = { 0.0f, 0.0f, 0.0f };

			cam.Aspect = 16.0f / 9.0f;
			cam.FovDegree = 45.0f;
			cam.NearPlane = 0.1f;
			cam.FarPlane = 1000.0f;
		}, "Camera");

		Entity cameraX = World().eEntity().Create<
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

				cam.Aspect = 16.0f / 9.0f;
				cam.FovDegree = 45.0f;
				cam.NearPlane = 0.1f;
				cam.FarPlane = 1000.0f;
			}, "CameraX");

		auto colorTexHandle = m_Framebuffer->Color()[0];
		auto depthTexHandle = m_Framebuffer->Depth();

		auto mat = make_scope<Material::MainQuad>();
		mat->TextureHandle = colorTexHandle;

		auto mainQuadMat = AssetManager::PutScope<Material::Base>(std::move(mat));

		auto quad = World().eEntity().Create<
			Component::Transform, Component::ModelMatrix, Component::RenderedModel
		>([mainQuadMat](auto&, auto&, Component::RenderedModel& model) -> void
		{
			model.Model = StaticModel::Quad(mainQuadMat);
			model.Layer = Component::RenderLayer::SCREEN_QUAD;
		}, "ScreenQuad");


		//AssetImporter<Model>::Params modelParams{ AssetManager::MODEL_DIR / "rem"  / "source" / "Victory.fbx", false, false, true };
		//auto modelHandle = AssetManager::Load<Model>(modelParams);
		//auto remModel = AssetManager::GetAsset(modelHandle).Asset;

		//AssetImporter<Model>::Params sponzaParams{ AssetManager::MODEL_DIR / "sponza_pbr" / "Sponza.gltf" };
		//auto sponzaHandle = AssetManager::Load<Model>(sponzaParams);
		//auto sponzaModel = AssetManager::GetAsset(sponzaHandle).Asset;

		//AssetImporter<Model>::Params dustParams{ AssetManager::MODEL_DIR / "dust2" / "de_dust2_d.gltf" };
		//auto dustHandle = AssetManager::Load<Model>(dustParams);
		//auto dustModel = AssetManager::GetAsset(dustHandle).Asset;

		AssetImporter<Model>::Params cacheParams{ AssetManager::MODEL_DIR / "dust2" / "de_dust2_d.gltf" };
		auto cacheHandle = AssetManager::Load<Model>(cacheParams);
		auto cacheModel = AssetManager::GetAsset(cacheHandle).Asset;
		
		//AssetImporter<Model>::Params gmodelParams{ AssetManager::MODEL_DIR / "gram" / "source" / "gram.fbx", false, false, false };
		//auto gramHandle = AssetManager::Load<Model>(gmodelParams);
		//auto gramModel = AssetManager::GetAsset(gramHandle).Asset;

		//scope<Material::PBRTextured> grammat = make_scope<Material::PBRTextured>();
		//grammat->AlbedoHandle = AssetManager::Load<Texture2D>(AssetManager::MODEL_DIR / "gram" / "textures" / "DefaultMaterial_Base_color.tga.png");
		//grammat->AOHandle = AssetManager::Load<Texture2D>(AssetManager::MODEL_DIR / "gram" / "textures" / "DefaultMaterial_Mixed_AO.tga.png");
		//grammat->MetallicHandle = AssetManager::Load<Texture2D>(AssetManager::MODEL_DIR / "gram" / "textures" / "DefaultMaterial_Metallic.tga.png");
		//grammat->RoughnessHandle = AssetManager::Load<Texture2D>(AssetManager::MODEL_DIR / "gram" / "textures" / "DefaultMaterial_Roughness.tga.png");
		//grammat->NormalsHandle = AssetManager::Load<Texture2D>( AssetManager::MODEL_DIR / "gram" / "textures" / "DefaultMaterial_Normal_OpenGL.tga.png");
		//gramModel->GetSubmesh(0).Material = AssetManager::PutScope<Material::Base>(std::move(grammat));

		//AssetImporter<Model>::Params wowModelParams{ AssetManager::MODEL_DIR / "wow" / "wow.fbx", false, false, true };
		//auto wowHandle = AssetManager::Load<Model>(wowModelParams);
		//auto wowModel = AssetManager::GetAsset(wowHandle).Asset;

		//for (u32 i = 1; i <= 10; i++)
		//{
		//	auto name = "Tower" + std::to_string(i);
		//	AssetImporter<Model>::Params towerParams{ AssetManager::MODEL_DIR / "towers" / name / (name + ".FBX")};
		//	auto towerHandle = AssetManager::Load<Model>(towerParams);

		//	World().eEntity().Create<Component::Transform, Component::ModelMatrix, Component::RenderedModel>(
		//		[=](Component::Transform& transform, auto&, Component::RenderedModel& model, auto&...) -> void
		//		{
		//			transform.Position = { i * 100.0f, 0.0f, 0.0f };
		//			transform.Scale = glm::vec3{ 1.0f };
		//			model.Model = towerHandle;
		//			model.MeshIdx = -1;
		//		}, name
		//	);
		//}


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

		//auto m1 = World().eEntity().Create<Component::Transform, Component::ModelMatrix, Component::RenderedModel, Component::LocalModelMatrix>(
		//	[=](Component::Transform& transform, auto&, Component::RenderedModel& model, auto&...) -> void
		//	{
		//		model.Model = modelHandle;
		//	}, "Rem"
		//);

		//auto sponza = World().eEntity().Create<Component::Transform, Component::ModelMatrix, Component::RenderedModel, Component::LocalModelMatrix>(
		//	[=](Component::Transform& transform, auto&, Component::RenderedModel& model, auto&...) -> void
		//	{
		//		model.Model = sponzaHandle;
		//	}, "sponza"
		//);

		//auto dust = World().eEntity().Create<Component::Transform, Component::ModelMatrix, Component::RenderedModel, Component::LocalModelMatrix>(
		//	[=](Component::Transform& transform, auto&, Component::RenderedModel& model, auto&...) -> void
		//	{
		//		model.Model = dustHandle;
		//	}, "dust"
		//); 
		
		auto cache = World().eEntity().Create<Component::Transform, Component::ModelMatrix, Component::RenderedModel, Component::LocalModelMatrix>(
			[=](Component::Transform& transform, auto&, Component::RenderedModel& model, auto&...) -> void
			{
				model.Model = cacheHandle;
			}, "cache"
		);

		//auto gm1 = World().eEntity().Create<Component::Transform, Component::ModelMatrix, Component::RenderedModel, Component::LocalModelMatrix>(
		//	[=](Component::Transform& transform, auto&, Component::RenderedModel& model, auto&...) -> void
		//	{
		//		model.Model = gramHandle;
		//	}, "gram"
		//);

		//auto wow = World().eEntity().Create<Component::Transform, Component::ModelMatrix, Component::RenderedModel, Component::LocalModelMatrix>(
		//	[=](Component::Transform& transform, auto&, Component::RenderedModel& model, auto&...) -> void
		//	{
		//		model.Model = wowHandle;
		//	}, "wow"
		//);

		//Component::AddAnimationData(World().eEntity(), wow, wowModel->Animations()[0], wowModel->Skeletons()[0]);
		//Component::AddAnimationData(World().eEntity(), m1, remModel->Animations()[0], remModel->Skeletons()[0]);


		auto dirLight = World().eEntity().Create<Component::Transform, Component::DirectionalLight, Component::ConstantRotation>(
			[=](Component::Transform& transform, Component::DirectionalLight& l, Component::ConstantRotation& rot) -> void
			{
				l.Color = glm::vec3{ 1.0f };
				rot.Velocity = glm::vec3{0.0f, 0.0f, 0.0f};
				transform.Rotation = glm::quat{ 0.308f, -0.075f, -0.833f, -0.453f };
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
		context.Framebuffer.MainFramebuffer = m_Framebuffer.get();

		f64 timeLastFrame = 0.0f;
		
		context.Cubemap.Asset = cubemapHadle;
		context.Cubemap.Id = cubemapEntity;
		context.Renderer.DepthWithoutTransparents = AssetManager::Put<Texture2D>(
			Texture2D{
				context.Framebuffer.MainFramebuffer->Resolution().x, context.Framebuffer.MainFramebuffer->Resolution().y,
				Texture::CreationInfoFromData{
					.CpuFormat = GL_DEPTH_COMPONENT,
					.GpuFormat = GL_DEPTH_COMPONENT32F,
					.DataType = GL_FLOAT
				}
			}
		);

		std::vector<Framebuffer> shadowFbs;
		shadowFbs.reserve(GameContext::RendererInfo::CascadedShadowMapInfo::NShadowMapCascades);

		for (u32 i = 0; i < GameContext::RendererInfo::CascadedShadowMapInfo::NShadowMapCascades; i++)
		{
			Framebuffer::CreationInfo dinfo;
			dinfo.DepthAttachment = AssetManager::Put<Texture2D>(
				Texture2D{
					context.Renderer.CascadedShadowMap.TextureSize[i], context.Renderer.CascadedShadowMap.TextureSize[i],
					Texture::CreationInfoFromData{
						.CpuFormat = GL_DEPTH_COMPONENT,
						.GpuFormat = GL_DEPTH_COMPONENT32F,
						.DataType = GL_FLOAT,
						.Common = {.Wrap = GL_CLAMP_TO_BORDER, .MinFilter = GL_NEAREST, .MagFilter = GL_NEAREST, .BorderColor = glm::vec4{1.0f}}
					}
				}
			);
			shadowFbs.emplace_back( dinfo );
			context.Renderer.CascadedShadowMap.DirShadowFramebuffer[i] = &shadowFbs[i];
			

			Entity cameraI = World().eEntity().Create<
				Component::Transform,
				Component::Velocity,
				Component::ModelMatrix,
				Component::ViewMatrix,
				Component::ProjectionMatrix,
				Component::OrthogonalProjection
			>([](Component::Transform& transform, Component::Velocity& mov, auto&, auto&, auto&, Component::OrthogonalProjection& cam)
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
				}, "Camera" + std::to_string(i));
			context.Camera.Cameras.insert({ cameraI, {} });
			context.Renderer.CascadedShadowMap.CascadeCamera[i] = cameraI;
		}

		context.DirLight.Id = dirLight;
		context.Camera.Cameras.insert({ camera, {.CanRotate = true } });
		context.Camera.Cameras.insert({ cameraX, {.CanRotate = true } });
		context.Camera.SetCamera(camera);
		context.Camera.SetTargetCamera(camera);

		World().eSystem<GameContext>()
			.Add<System::Input>()
			.Add<System::PlayAnimation>()
			.Add<System::Movement>()
			.Add<System::ConstantRotation>()
			.Add<System::FollowSnap>()
			.Add<System::ModelMatrix>()
			.Add<System::SceneGraph>()
			.Add<System::CollectAnimationTransforms>()
			.Add<System::ViewMatrix>()
			.Add<System::ProjectionMatrix>()
			.Add<System::UpdateCameras>()
			.Add<System::CollisionDetection>()
			.Add<System::Gravity>()
			.Add<System::CollectLights>()
			.Add<System::CascadedShadowMapFit>()
			.Add<System::WaterSystem>()
			.Add<System::PrepareDebugDraw>()
			.Add<System::BundleDirtyMaterials>()
			.Add<System::ResetRenderState>()
			.Add<System::Prepass>()
			.Add<System::CascadedShadowMapRender>()
			.Add<System::SSAO>()
			.Add<System::Render>()
			.Add<System::DrawDebug>()
			.Add<System::TerrainSystem>()
			.Add<System::CloudSystem>()
			.Add<System::UpdateCubemap>()
			.Add<System::RenderScreenQuad>()
			.Add<System::Cleanup>()
			.Init();

		context.FrameStats.resize(World().eSystem<GameContext>().Systems().size());

		for (auto& stat : context.FrameStats)
		{
			GL_CHECK(glGenQueries(1, &stat.GpuQueryIndex));
		}

		while (!glfwWindowShouldClose(m_Window))
		{
			glfwPollEvents();
;
			InputHandler::ProcessInputEvents(m_Window);
				
			f64 time = glfwGetTime();
			f32 deltaTime = static_cast<f32>(time - timeLastFrame);
			timeLastFrame = time;
			context.Time = time;
			context.DeltaTime = deltaTime;

			if (!Global::IsPaused)
			{
				u32 i = 0u;
				World().eSystem<GameContext>().Run(context, [&](auto&& func) 
					{
						auto start = glfwGetTime();

							GL_CHECK(glBeginQuery(GL_TIME_ELAPSED, context.FrameStats[i].GpuQueryIndex));
								func();
							GL_CHECK(glEndQuery(GL_TIME_ELAPSED));

						auto end = glfwGetTime();

						context.FrameStats[i++].TimeCpu = static_cast<f32>((end - start) * 1000.0);
					}
				);
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