#pragma once

#include "ecs/systems/Systems.h"
#include "algorithm/compute/noise/NoiseTexture.h"


namespace EnGl::System
{
	class CloudSystem : public SystemImpl
	{
	public:
		struct AtmosphereParams
		{
			glm::vec2 TransmittanceLUTRes{ 128, 32 };
		};

		struct Params
		{
			f32 ResScale = 4.0f;
			
			f32 CloudScale = 0.54f;
			f32 DetailScale = 2.0f;
			f32 WeatherMapScale = 4.1f;
			
			f32 PlanetRadius = 200000.0f;

			f32 SkyHeightMin = 1000.0f;
			f32 SkyHeightMax = 3000.0f;

			u32 NumRaySteps = 128u;
			u32 NumRayStepsLight = 4u;

			glm::vec3 Direction = glm::vec3{ 1, 0, 1 };
			f32 Speed = 30.0f;

			f32 Anvil = 0.5f;
			
			f32 GlobalCoverage = 0.58f;
			f32 GlobalOpacity = 0.005f;

			f32 ShapeBottomCut = 0.07f;
			f32 DensityBottomCut = 0.2f;
			f32 DensityTopCut = 0.9f;

			f32 Beer = 6.0f;
			f32 InScatter = 0.2f;
			f32 OutScatter = 0.1f;
			f32 OutScatterAmbient = 0.9f;
			f32 InOutCoefficient = 0.5f;
			f32 SilverLine = 2.5f;
			f32 SilverLineExp = 2.0f;
			f32 Ambient = 0.00004f;

			glm::vec3 ExtinctionColor = glm::vec3{ 0, 0, 0 };

			Noise3DWrapper Shape
			{
				.Channels = 
				{
					Noise3DChannel{.Params = {.NCells = 1u, .NChannel = 0u, .Seed = 0u, .Octaves = 8u}, .Strategy = Noise3D::PerlinWorley},
					Noise3DChannel{.Params = {.NCells = 8u, .NChannel = 1u, .Seed = 133u, .Octaves = 8u}, .Strategy = Noise3D::Worley},
					Noise3DChannel{.Params = {.NCells = 16u, .NChannel = 2u, .Seed = 157u, .Octaves = 8u}, .Strategy = Noise3D::Worley},
					Noise3DChannel{.Params = {.NCells = 32u, .NChannel = 3u, .Seed = 17u, .Octaves = 8u}, .Strategy = Noise3D::Worley}
				},
				.Texture = AssetManager::Put<Texture3D>(128u, Texture::CreationInfoFromData{.CpuFormat = GL_RGBA, .GpuFormat = GL_RGBA32F })
			};

			Noise3DWrapper Detail
			{
				.Channels =
				{
					Noise3DChannel{.Params = {.NCells = 1u, .NChannel = 0u, .Seed = 0u, .Octaves = 8u}, .Strategy = Noise3D::Worley},
					Noise3DChannel{.Params = {.NCells = 4u, .NChannel = 1u, .Seed = 123u, .Octaves = 8u}, .Strategy = Noise3D::Worley},
					Noise3DChannel{.Params = {.NCells = 8u, .NChannel = 2u, .Seed = 187u, .Octaves = 8u}, .Strategy = Noise3D::Worley},
					Noise3DChannel{.Params = {.NCells = 16u, .NChannel = 3u, .Seed = 13u, .Octaves = 8u}, .Strategy = Noise3D::Worley}
				},
				.Texture = AssetManager::Put<Texture3D>(32u, Texture::CreationInfoFromData{.CpuFormat = GL_RGBA, .GpuFormat = GL_RGBA32F })
			};

			Noise2DWrapper Weather
			{
				.Channels =
				{
					Noise2DChannel{.Params = {.NCells = 8u, .NChannel = 0u, .Seed = 0u, .Octaves = 1u, .DarkThreshold = 0.001f }, .Strategy = Noise2D::Worley},
					//Noise2DChannel{.Params = {.NCells = 2u, .NChannel = 0u, .Seed = 123u, .Octaves = 8u}, .Strategy = Noise2D::Perlin},
					//Noise2DChannel{.Params = {.NChannel = 0u, .DarkThreshold = 1.0f}, .Strategy = Noise2D::Const},
					Noise2DChannel{.Params = {.NChannel = 1u, .DarkThreshold = 1.0f}, .Strategy = Noise2D::Const},
					Noise2DChannel{.Params = {.NCells = 2u, .NChannel = 2u, .Seed = 187u, .Octaves = 8u, .DarkThreshold = -1.0f}, .Strategy = Noise2D::Perlin},
					Noise2DChannel{.Params = {.NCells = 2u, .NChannel = 3u, .Seed = 13u, .Octaves = 1u}, .Strategy = Noise2D::Worley}
				},
				.Texture = AssetManager::Put<Texture2D>(512u, Texture::CreationInfoFromData{.CpuFormat = GL_RGBA, .GpuFormat = GL_RGBA32F })
			};
		};

		void Init(EcsImpl::EntityManager& manager) override;
		void Run(EcsImpl::EntityManager& manager, GameContext& context) override;
		void Editor(EcsImpl::EntityManager& manager, GameContext& context) override;
		void PrecomputeTransmittance();
		void PrecomputeAtmosphere();

		inline AssetHandle<Texture2D> Sky() const { return m_SkyTexture; }

		AtmosphereParams m_AtmosphereParams;
	private:
		AssetHandle<ComputeShader> m_LowResShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "CloudsLowRes");
		AssetHandle<ComputeShader> m_UpsampleShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "CloudsUpsample");
		AssetHandle<ComputeShader> m_BlurShaderV = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "BlurV");
		AssetHandle<ComputeShader> m_BlurShaderH = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "BlurH");
		AssetHandle<ComputeShader> m_EdgeShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "Edge");
		AssetHandle<ComputeShader> m_FullResShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "CloudsEdgeFill");
		AssetHandle<Texture2D> m_SkyTextureLowResA;
		AssetHandle<Texture2D> m_SkyTextureLowResB;
		AssetHandle<Texture2D> m_SkyTexture;
		AssetHandle<Texture2D> m_HistoryTexture;
		AssetHandle<Texture2D> m_EdgeTexture;

		AssetHandle<ComputeShader> m_TransmittanceLUTShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "TRANSMITTANCELUT");
		AssetHandle<Texture2D> m_TransmittanceLUT;
		glm::uvec2 m_Res{ 0 };

		struct CloudTextures
		{
			Texture2D* LowResA = nullptr;
			Texture2D* LowResB = nullptr;
			Texture2D* Sky = nullptr;
			Texture2D* History = nullptr;
			Texture2D* Edge = nullptr;
		};

		static const std::vector<glm::vec2> CrossOffset4x4;
		CloudTextures GetTextures(GameContext& context);
		void DispatchLowRes(GameContext& context, CloudTextures& texLowRes);
		void DispatchUpsample(GameContext& context, CloudTextures& textures);
		void DispatchBlur(GameContext& context, CloudTextures& textures);
		void DispatchEdge(GameContext& context, CloudTextures& textures);
		void DispatchFullRes(GameContext& context, CloudTextures& textures);
		void PresetSelect(EcsImpl::EntityManager& manager, GameContext& context);

		u32 m_CurrentRenderOffsetIdx = 0;

		struct Preset
		{
			Params CloudParams;
			f32 CameraFar;
			glm::vec3 DirLightColor;
			AssetImporter<Cubemap>::Params Cubemap;
		};

		Params m_DayLarge{ .GlobalOpacity = 0.010f, .OutScatterAmbient = 1.0f };
		Params m_Night{ .CloudScale = 1.43f, .DetailScale = 10.0f,  .WeatherMapScale = 29.24f, .PlanetRadius = 52960.0f, .SkyHeightMin = 450.0f, .SkyHeightMax = 1000.0f, .GlobalCoverage = 0.644f, .GlobalOpacity = 0.285f, .Beer = 4.0f, .InScatter = 0.293f, .OutScatter = 0.949f, .InOutCoefficient = 0.652f, .SilverLine = 4.674f, .SilverLineExp = 2.863f, .Ambient = 0.00009f, .ExtinctionColor = glm::vec3{7.0f / 255.0f, 10.0f / 255.0f, 30.0f / 255.0f} };
		Params m_DaySmall{ .CloudScale = 1.63f, .DetailScale = 5.85f,  .WeatherMapScale = 28.77f, .PlanetRadius = 20000.0f, .SkyHeightMin = 450.0f, .SkyHeightMax = 1000.0f, .GlobalCoverage = 0.644f, .GlobalOpacity = 0.01f, .OutScatterAmbient = 1.0f, .ExtinctionColor = glm::vec3{13.0f / 255.0f, 12.0f / 255.0f, 64.0f / 255.0f} };
		Params m_DaySmallSunrise{ .CloudScale = 1.63f, .DetailScale = 5.85f,  .WeatherMapScale = 28.77f, .PlanetRadius = 20000.0f, .SkyHeightMin = 450.0f, .SkyHeightMax = 1000.0f, .GlobalCoverage = 0.644f, .GlobalOpacity = 0.081f, .OutScatterAmbient = 1.0f, .SilverLine = 11.5f, .ExtinctionColor = glm::vec3{94.0f / 255.0f, 54.0f / 255.0f, 23.0f / 255.0f} };
		Params m_DaySmallAlt{ .CloudScale = 0.51f, .DetailScale = 5.85f,  .WeatherMapScale = 28.77f, .PlanetRadius = 20000.0f, .SkyHeightMin = 450.0f, .SkyHeightMax = 1000.0f, .GlobalCoverage = 0.327f, .GlobalOpacity = 0.019f, .OutScatterAmbient = 1.0f, .ExtinctionColor = glm::vec3{0.0f}
			, .Weather = Noise2DWrapper{
				.Channels =
				{
					Noise2DChannel{.Params = {.NChannel = 0u, .DarkThreshold = 1.0f}, .Strategy = Noise2D::Const},
					Noise2DChannel{.Params = {.NChannel = 1u, .DarkThreshold = 1.0f}, .Strategy = Noise2D::Const},
					Noise2DChannel{.Params = {.NCells = 2u, .NChannel = 2u, .Seed = 187u, .Octaves = 8u}, .Strategy = Noise2D::Perlin},
					Noise2DChannel{.Params = {.NCells = 16u, .NChannel = 3u, .Seed = 13u, .Octaves = 8u}, .Strategy = Noise2D::Worley}
				},
				.Texture = AssetManager::Put<Texture2D>(512u, Texture::CreationInfoFromData{.CpuFormat = GL_RGBA, .GpuFormat = GL_RGBA32F })
			}
		};
		Params m_DaySmallAltSunset{ .CloudScale = 0.51f, .DetailScale = 5.85f,  .WeatherMapScale = 28.77f, .PlanetRadius = 2000.0f, .SkyHeightMin = 450.0f, .SkyHeightMax = 1000.0f, .GlobalCoverage = 0.327f, .GlobalOpacity = 0.019f, .OutScatterAmbient = 1.0f, .ExtinctionColor = glm::vec3{12.0f / 255.0f, 26.0f / 255.0f, 69.0f / 255.0f}
			, .Weather = Noise2DWrapper{
				.Channels =
				{
					Noise2DChannel{.Params = {.NChannel = 0u, .DarkThreshold = 1.0f}, .Strategy = Noise2D::Const},
					Noise2DChannel{.Params = {.NChannel = 1u, .DarkThreshold = 1.0f}, .Strategy = Noise2D::Const},
					Noise2DChannel{.Params = {.NCells = 2u, .NChannel = 2u, .Seed = 187u, .Octaves = 8u}, .Strategy = Noise2D::Perlin},
					Noise2DChannel{.Params = {.NCells = 16u, .NChannel = 3u, .Seed = 13u, .Octaves = 8u}, .Strategy = Noise2D::Worley}
				},
				.Texture = AssetManager::Put<Texture2D>(512u, Texture::CreationInfoFromData{.CpuFormat = GL_RGBA, .GpuFormat = GL_RGBA32F })
			}
		};
		Params m_DayMedium{ .CloudScale = 0.81f, .DetailScale = 10.0f,  .WeatherMapScale = 9.0f, .PlanetRadius = 5709.375f, .SkyHeightMin = 450.0f, .SkyHeightMax = 1000.0f, .Speed = 10.0f, .GlobalCoverage = 0.697f, .GlobalOpacity =  0.019f, .OutScatterAmbient = 1.0f, .ExtinctionColor = glm::vec3{0.0f} };

		Params m_Params = m_DayLarge;

		AssetImporter<Cubemap>::Params m_MidnightCubemap
		{
		{
			AssetManager::TEXTURE_DIR / "skybox" / "midnight" / "r.png",
			AssetManager::TEXTURE_DIR / "skybox" / "midnight" / "l.png",
			AssetManager::TEXTURE_DIR / "skybox" / "midnight" / "t.png",
			AssetManager::TEXTURE_DIR / "skybox" / "midnight" / "d.png",
			AssetManager::TEXTURE_DIR / "skybox" / "midnight" / "f.png",
			AssetManager::TEXTURE_DIR / "skybox" / "midnight" / "b.png"
		},
			{false, false, false, false, false, false} 
		};

		AssetImporter<Cubemap>::Params m_DayCubemap
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

		AssetImporter<Cubemap>::Params m_SunsetCubemap
		{
		{
			AssetManager::TEXTURE_DIR / "skybox" / "sunset" / "r.png",
			AssetManager::TEXTURE_DIR / "skybox" / "sunset" / "l.png",
			AssetManager::TEXTURE_DIR / "skybox" / "sunset" / "t.png",
			AssetManager::TEXTURE_DIR / "skybox" / "sunset" / "d.png",
			AssetManager::TEXTURE_DIR / "skybox" / "sunset" / "f.png",
			AssetManager::TEXTURE_DIR / "skybox" / "sunset" / "b.png"
		}
		};

		std::unordered_map<std::string, Preset> m_Presets
		{ 
			{ "Day large", Preset{.CloudParams = m_DayLarge, .CameraFar = 1000.0f, .DirLightColor = glm::vec3{1.0f}, .Cubemap = m_DayCubemap }},
			{ "Day medium", Preset{.CloudParams = m_DayMedium, .CameraFar = 1000.0f, .DirLightColor = glm::vec3{1.0f}, .Cubemap = m_DayCubemap }},
			{ "Night", Preset{.CloudParams = m_Night, .CameraFar = 1000.0f, .DirLightColor = glm::vec3{65.0f / 255.0f, 138.0f / 255.0f, 1.0f}, .Cubemap = m_MidnightCubemap } },
			{ "Day small", Preset{.CloudParams = m_DaySmall, .CameraFar = 1000.0f, .DirLightColor = glm::vec3{255.0f / 255.0f, 235.0f / 255.0f, 169.0f / 255.0f}, .Cubemap = m_DayCubemap } },
			{ "Day small sunrise", Preset{.CloudParams = m_DaySmallSunrise, .CameraFar = 1000.0f, .DirLightColor = glm::vec3{228.0f / 255.0f, 196.0f / 255.0f, 91.0f / 255.0f}, .Cubemap = m_DayCubemap } },
			{ "Day alt weather map small", Preset{.CloudParams = m_DaySmallAlt, .CameraFar = 1000.0f, .DirLightColor = glm::vec3{255.0f / 255.0f, 235.0f / 255.0f, 169.0f / 255.0f}, .Cubemap = m_DayCubemap } },
			{ "Day alt weather map small sunset", Preset{.CloudParams = m_DaySmallAltSunset, .CameraFar = 1000.0f, .DirLightColor = glm::vec3{255.0f / 255.0f, 169.0f / 255.0f, 169.0f / 255.0f}, .Cubemap = m_DayCubemap } }
		};
	};
}
