#include "FFT.h"
#include <unordered_map>


namespace EnGl
{
	AssetHandle<Texture2D> FFT::GetButterfly(u32 nstages, u32 N) const
	{
		static std::unordered_map<u64, AssetHandle<Texture2D>> butterflyMap;

		const u64 hash = static_cast<u64>(nstages) | (static_cast<u64>(N) >> 32);
		if (!butterflyMap.contains(hash))
		{
			auto [shader, gen] = AssetManager::GetAsset(m_ButterflyShader);
			if (shader)
			{
				Texture2D butterflyTex = Texture2D{ nstages, N, Texture::CreationInfoFromData{.CpuFormat = GL_RGBA, .GpuFormat= GL_RGBA32F } };

				shader->Use();
				shader->SetUniform("N", N);
				shader->SetUniform("bitcount", nstages);

				shader->BindWriteTexture(butterflyTex, 0);
				shader->DispatchWait({ nstages, N }, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

				auto texHandle = AssetManager::Put(std::move(butterflyTex));
				butterflyMap.insert({ hash, texHandle });
			}
		}

		return butterflyMap.at(hash);
	}

	AssetHandle<Texture2D> FFT::IFFT2D(IFFT2Dinfo& info) const
	{
		auto butterflyTexHandle = GetButterfly(info.Nstages, info.N);
		auto [butterflyTex, g0] = AssetManager::GetAsset(butterflyTexHandle);

		auto [horizontal, g1] = AssetManager::GetAsset(m_HorizontalPassShader);
		auto [vertical, g2] = AssetManager::GetAsset(m_VerticalPassShader);
		auto [permutation, g3] = AssetManager::GetAsset(m_PermutationShader);

		auto [ping, g4] = AssetManager::GetAsset(info.Ping);
		auto [pong, g5] = AssetManager::GetAsset(info.Pong);

		if (!butterflyTex)
		{
			spdlog::error("Butterfly texture for fft is not loaded.");
			return info.Ping;
		}

		if (!ping || !pong)
		{
			spdlog::error("Ping|Pong input for fft is not loaded.");
			return info.Ping;
		}

		if (!horizontal || !vertical || !permutation)
		{
			spdlog::error("Shaders for fft are not loaded.");
			return info.Ping;
		}

		horizontal->Use();
		horizontal->BindReadTexture(*butterflyTex, 0);
		horizontal->BindReadWriteTexture(*ping, 1);
		horizontal->BindReadWriteTexture(*pong, 2);

		bool pingpong = false;

		for (u32 i = 0; i < info.Nstages; i++)
		{
			horizontal->SetUniform("stage", i);
			horizontal->SetUniform("pingpong", pingpong);

			horizontal->DispatchWait({ info.GroupCount, info.GroupCount }, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			pingpong = !pingpong;
		}

		vertical->Use();
		vertical->BindReadTexture(*butterflyTex, 0);
		vertical->BindReadWriteTexture(*ping, 1);
		vertical->BindReadWriteTexture(*pong, 2);

		for (u32 i = 0; i < info.Nstages; i++)
		{
			vertical->SetUniform("stage", i);
			vertical->SetUniform("pingpong", pingpong);

			vertical->DispatchWait({ info.GroupCount, info.GroupCount }, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

			pingpong = !pingpong;
		}

		permutation->Use();
		permutation->SetUniform("N", info.N);
		permutation->SetUniform("pingpong", pingpong);
		permutation->BindReadWriteTexture(*ping, 0);
		permutation->BindReadWriteTexture(*pong, 1);

		permutation->DispatchWait({ info.GroupCount, info.GroupCount }, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		return pingpong ? info.Ping : info.Pong;
	}
}

