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

			f32 CloudScale = 0.432f;
			f32 DetailScale = 4.866f;
			f32 WeatherMapScale = 1.106f;

			f32 SkySpan = 30000.0f;
			f32 SkyHeightMin = 1000.0f;
			f32 SkyHeightMax = 3000.0f;

			u32 NumRaySteps = 100;
			u32 NumRayStepsLight = 4;

			glm::vec3 Direction = glm::vec3{ 1, 0, 1 };
			f32 Speed = 30.0f;

			f32 DarknessThreshold = 0.2f;

			f32 LightAbsorptionSun = 1.0f;
			f32 LightAbsorptionCloud = 1.0f;

			f32 PhaseVal = 0.8f;

			f32 GlobalCoverage = 0.474f;
			f32 GlobalOpacity = 0.134f;

			f32 Anvil = 0.5f;

			//struct Noise3DChannel
			//{
			//	NoiseParams Params{};
			//	const Noise3D& Strategy = NoiseSingleton<PerlinNoise3D>();
			//};

			//constexpr static u32 NChannels = 4u;

			//struct Noise3DWrapper
			//{
			//	Noise3DChannel Channels[NChannels]{};
			//	AssetHandle<Texture3D> Texture = AssetManager::Put<Texture3D>(128u, Texture::CreationInfoFromData{ .CpuFormat = GL_RGBA, .GpuFormat = GL_RGBA32F });
			//};

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
					Noise2DChannel{.Params = {.NCells = 32u, .NChannel = 0u, .Seed = 0u, .Octaves = 1u, .DarkThreshold = 0.001f }, .Strategy = Noise2D::Worley},
					Noise2DChannel{.Params = {.NCells = 2u, .NChannel = 1u, .Seed = 123u, .Octaves = 8u}, .Strategy = Noise2D::Perlin},
					Noise2DChannel{.Params = {.NCells = 16u, .NChannel = 2u, .Seed = 187u, .Octaves = 8u}, .Strategy = Noise2D::Worley},
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
		AssetHandle<ComputeShader> m_Shader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "CLOUDS");
		AssetHandle<Texture2D> m_SkyTexture;
		AssetHandle<Texture2D> m_SkyDepth;

		AssetHandle<ComputeShader> m_TransmittanceLUTShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "TRANSMITTANCELUT");
		AssetHandle<Texture2D> m_TransmittanceLUT;
		glm::uvec2 m_Res{ 0 };

		static const std::vector<glm::vec2> CrossOffset4x4;

		u32 m_CurrentRenderOffsetIdx = 0;
		const std::vector<glm::vec2>& m_CurrentRenderOffset = CrossOffset4x4;
	};
}
