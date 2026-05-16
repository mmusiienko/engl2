#pragma once

#include "ecs/systems/Systems.h"
#include "algorithm/compute/noise/NoiseTexture.h"


namespace EnGl
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

			/*f32 CloudScale = 0.18f;
			f32 DetailScale = 1.129f;
			f32 WeatherMapScale = 1.791f;*/

			/*f32 CloudScale = 0.18f;
			f32 DetailScale = 1.129f;
			f32 WeatherMapScale = 1.264f; */
			
			f32 CloudScale = 0.054f;
			f32 DetailScale = 0.2f;
			f32 WeatherMapScale = 0.411f;
			
			f32 PlanetRadius = 6000000.0f;

			/*f32 CloudScale = 0.162f;
			f32 DetailScale = 1.147f;
			f32 WeatherMapScale = 0.136f;*/

			f32 SkySpan = 30000.0f;
			f32 SkyHeightMin = 10000.0f;
			f32 SkyHeightMax = 30000.0f;

			u32 NumRaySteps = 128u;
			u32 NumRayStepsLight = 4u;

			glm::vec3 Direction = glm::vec3{ 1, 0, 1 };
			f32 Speed = 300.0f;

			f32 DarknessThreshold = 0.2f;

			f32 LightAbsorptionSun = 1.0f;
			f32 LightAbsorptionCloud = 1.0f;

			f32 PhaseVal = 0.8f;

			f32 Anvil = 0.5f;

			/*f32 GlobalCoverage = 0.455f;
			f32 GlobalOpacity = 0.01f; */
			
			f32 GlobalCoverage = 0.58f;
			f32 GlobalOpacity = 0.005f;

			f32 ShapeBottomCut = 0.07f;
			f32 DensityBottomCut = 0.2f;
			f32 DensityTopCut = 0.9f;

			f32 Beer = 6.0f;
			f32 InScatter = 0.2f;
			f32 OutScatter = 0.1f;
			f32 InOutCoefficient = 0.5f;
			f32 SilverLine = 2.5f;
			f32 SilverLineExp = 2.0f;
			f32 Ambient = 0.429f;

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
					Noise2DChannel{.Params = {.NCells = 2u, .NChannel = 2u, .Seed = 187u, .Octaves = 8u}, .Strategy = Noise2D::Perlin},
					Noise2DChannel{.Params = {.NCells = 16u, .NChannel = 3u, .Seed = 13u, .Octaves = 8u}, .Strategy = Noise2D::Worley}
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

		Params m_Params;
		AtmosphereParams m_AtmosphereParams;
	private:
		AssetHandle<ComputeShader> m_LowResShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "CloudsLowRes");
		AssetHandle<ComputeShader> m_UpsampleShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "CloudsUpsample");
		AssetHandle<ComputeShader> m_DepthNeighborShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "CloudsDepthNeighborFill");
		AssetHandle<ComputeShader> m_BlurShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "CloudsBlur");
		AssetHandle<ComputeShader> m_EdgeShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "Edge");
		AssetHandle<ComputeShader> m_FullResShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "CloudsEdgeFill");
		AssetHandle<Texture2D> m_SkyTextureLowResA;
		AssetHandle<Texture2D> m_SkyTextureLowResB;
		AssetHandle<Texture2D> m_SkyTexture;
		AssetHandle<Texture2D> m_HistoryTexture;

		AssetHandle<ComputeShader> m_TransmittanceLUTShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "TRANSMITTANCELUT");
		AssetHandle<Texture2D> m_TransmittanceLUT;
		glm::uvec2 m_Res{ 0 };

		struct CloudTextures
		{
			Texture2D* LowResA = nullptr;
			Texture2D* LowResB = nullptr;
			Texture2D* Sky = nullptr;
			Texture2D* History = nullptr;
		};

		static const std::vector<glm::vec2> CrossOffset4x4;
		CloudTextures GetTextures(GameContext& context);
		void DispatchLowRes(GameContext& context, CloudTextures& texLowRes);
		void DispatchUpsample(GameContext& context, CloudTextures& textures);
		void DispatchDepthNeighborFill(GameContext& context, CloudTextures& textures);
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
		};

		Params m_SmallScaleSphere{ .PlanetRadius = 100000.0f };

		std::unordered_map<std::string, Preset> m_Presets
		{ 
			{ "Small scale sphere", Preset{ .CloudParams = m_SmallScaleSphere, .CameraFar = 50000.0f, .DirLightColor = glm::vec3{1.0f} } }
		};
	};
}
