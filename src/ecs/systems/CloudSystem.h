#include "./Systems.h"
#include "../algorithm/compute/noise/Fractal.h"
#include "../algorithm/compute/noise/Perlin.h"
#include "../algorithm/compute/noise/Voronoi.h"


namespace EnGl
{
	class CloudSystem : public SystemImpl
	{
	public:
		struct Params
		{
			f32 ResScale = 4.0f;

			glm::vec3 CloudScale = glm::vec3{ 0.2f };
			glm::vec3 DetailScale = glm::vec3{ 1 };

			f32 SkySpan = 30000.0f;
			f32 SkyHeightMin = 0.0f;
			f32 SkyHeightMax = 1500.0f;

			u32 NumRaySteps = 100;
			u32 NumRayStepsLight = 4;

			glm::vec3 Direction = glm::vec3{ 1, 0, 1 };
			f32 Speed = 30.0f;

			f32 DarknessThreshold = 0.5f;

			f32 LightAbsorptionSun = 1.0f;
			f32 LightAbsorptionCloud = 1.0f;

			f32 PhaseVal = 0.8f;

			f32 GlobalCoverage = 0.476f;
			f32 GlobalOpacity = 0.01f;

			VoronoiWrapper Voronoi1FBM{};
			VoronoiWrapper Voronoi2FBM{};
		};

		void Init(EcsImpl::EntityManager& manager) override;
		void Run(EcsImpl::EntityManager& manager, GameContext& context) override;
		void Editor(EcsImpl::EntityManager& manager, GameContext& context) override;

		inline AssetHandle<Texture2D> Sky() const { return m_SkyTexture; }

		Params m_Params;
	private:
		AssetHandle<ComputeShader> m_Shader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "CLOUDS");
		AssetHandle<Texture2D> m_SkyTexture;
		glm::vec2 m_Res { 0 };
	};
}
