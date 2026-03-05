#pragma once
#include "./Systems.h"
#include "../../algorithm/compute/FFT.h"
#include <vector>


namespace EnGl
{
	class WaterSystem : public SystemImpl
	{
	public:
		static constexpr u32 NCascades = 4;

		struct SpectrumData
		{
			f32 L = 1000;

			f32 Lambda = 2.0f;

			f32 TimeScale = 5.0f;

			f32 LowCutoff = 0.0f;
			f32 HighCutoff = 5000.0f;

			f32 Tiling = 1.0f;

			bool ParametersChanged = true;
		};

		struct CommonSpectrumData
		{
			u32 N = 256;

			f32 WindSpeed = 15;
			f32 WindAngleDegree = 45.0f;
			
			f32 Swell = 0.5f;
			f32 Fetch = 1000.0f * 1000;
			f32 Depth = 500;

			f32 FoamAdd = 0.7f;
			f32 FoamDecay = 0.055f;

			bool DimensionChanged = false;
		};

		static inline std::vector<SpectrumData> DEFAULT_DATA =
		{
			{
				.L = 3282.0f,

				.Lambda = 1.0f,

				.TimeScale = 2.0f,

				.LowCutoff = 0.0f,
				.HighCutoff = 0.05f,

				.Tiling = 1.0f
			},
			{
				.L = 655.0f,

				.Lambda = 1.0f,

				.TimeScale = 2.0f,

				.LowCutoff = 0.05f,
				.HighCutoff = 1.0f,

				.Tiling = 1.0f
			},
			{
				.L = 237.0f,

				.Lambda = 1.0f,

				.TimeScale = 2.0f,

				.LowCutoff = 1.0f,
				.HighCutoff = 2.0f,

				.Tiling = 1.0f
			},
			{
				.L = 71.0f,

				.Lambda = 1.0f,

				.TimeScale = 2.0f,

				.LowCutoff = 2.0f,
				.HighCutoff = 4.0f,

				.Tiling = 1.0f
			}
		};

		CommonSpectrumData m_Data;
		
		void Run(EcsImpl::EntityManager& manager, GameContext& context) override;
		void Init(EcsImpl::EntityManager& manager) override;
		void Editor(EcsImpl::EntityManager& manager, GameContext& context) override;

		class Cascade
		{
		public:
			SpectrumData& m_SpectrumData;
			CommonSpectrumData& m_CommonData;

			Cascade(SpectrumData& data, CommonSpectrumData& commonData, const FFT& fft);

			AssetHandle<Texture2D> m_Normal;
			AssetHandle<Texture2D> m_Displacement;
		private:
			const FFT& m_FFT;

			FFT::IFFT2Dinfo m_IFFTDYCOMBINEDDXZ;
			FFT::IFFT2Dinfo m_IFFTDXCOMBINEDZ;
			FFT::IFFT2Dinfo m_IFFTDDXCOMBINEDDZ;
			FFT::IFFT2Dinfo m_IFFTDYDXCOMBINEDYDZ;

			u32 m_NStages;
			u32 m_GroupCount;

			AssetHandle<Texture2D> m_Gaussian;
			AssetHandle<Texture2D> m_Spectrum;
			AssetHandle<Texture2D> m_ConjSpectrum;

			AssetHandle<Texture2D> m_dycombineddxz;
			AssetHandle<Texture2D> m_dxcombinedz;
			AssetHandle<Texture2D> m_ddxcombineddz;
			AssetHandle<Texture2D> m_dydxcombinedydz;

			AssetHandle<ComputeShader> m_SpectrumShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "FFTSPECTRUM2");
			AssetHandle<ComputeShader> m_ConjSpectrumShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "FFTSPECTRUMCONJUGATE");
			AssetHandle<ComputeShader> m_HtPassShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "HTPASS");
			AssetHandle<ComputeShader> m_CombineShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "COMBINE");
			AssetHandle<ComputeShader> m_BlurShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "BLUR");

			void CheckParametersChange();
			void UpdateTimeSpectrum(f64 time);
			void CombineResults(f32 dt);
			void Init(u32 N);
			void Update(f64 time, f32 dt);
			void ResizeAll();

			friend class WaterSystem;
		};

		WaterSystem(std::vector<SpectrumData> data = DEFAULT_DATA);

		FFT m_FFT;
		std::vector<SpectrumData> m_SpectrumData;
		std::vector<Cascade> m_Cascades;
	private:
		std::vector<Cascade> GenCascades();
		EcsImpl::Entity m_WaterSurface = 0;

		void CombineResults(f32 dt);
	};
}