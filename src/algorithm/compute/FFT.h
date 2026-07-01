#pragma once

#include "resources/importers/AssetManager.h"
#include "renderer/base/Texture.h"
#include "renderer/base/ComputeShader.h"


namespace EnGl
{
	class FFT
	{
	public:
		FFT() = default;

		struct IFFT2Dinfo
		{
			AssetHandle<Texture2D> Ping{};
			AssetHandle<Texture2D> Pong{};
			u32 N = 0u;
			u32 Nstages = 0u;
			u32 GroupCount = 0u;
		};

		AssetHandle<Texture2D> IFFT2D(IFFT2Dinfo& info) const;
	private:
		AssetHandle<Texture2D> GetButterfly(u32 nstages, u32 N) const;
		
		AssetHandle<ComputeShader> m_ButterflyShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "BUTTERFLY");
		AssetHandle<ComputeShader> m_HorizontalPassShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "FFTHORIZONTAL");
		AssetHandle<ComputeShader> m_VerticalPassShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "FFTVERTICAL");
		AssetHandle<ComputeShader> m_PermutationShader = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "FFTPERMUTE");
	};
}