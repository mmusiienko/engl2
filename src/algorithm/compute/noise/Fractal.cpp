#include "Fractal.h"
#include "../../core/Global.h"


namespace EnGl
{
	void FractalNoise2D::Fill(AssetHandle<Texture2D> textureA, AssetHandle<Texture2D> baseNoise, const FractalNoiseParams& fParams)
	{
		static AssetHandle<ComputeShader> shaderHandle = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "FRACTAL2D");

		auto [shader, g] = AssetManager::GetAsset(shaderHandle);
		auto [texture, g1] = AssetManager::GetAsset(textureA);
		auto [base, g2] = AssetManager::GetAsset(baseNoise);

		if (!shader)
		{
			spdlog::error("Shader for perlin noise is not loaded.");
			return;
		}

		if (!texture || !base)
		{
			spdlog::error("Texture for perlin noise is not loaded.");
			return;
		}

		shader->BindTextureUnit(*base, 0);
		shader->BindWriteTexture(*texture, 1);

		shader->Use();

		shader->SetUniform("w", base->Properties().w);
		shader->SetUniform("h", base->Properties().h);

		shader->SetUniform("channelFrom", std::clamp(fParams.ChannelFrom, 1u, 4u));
		shader->SetUniform("channelTo", std::clamp(fParams.ChannelTo, 1u, 4u));

		shader->SetUniform("N", fParams.NOctaves);
		shader->SetUniform("persistence", fParams.Persistence);
		shader->SetUniform("lacunarity", fParams.Lacunarity);

		shader->DispatchWait(
			{
				static_cast<u32>(glm::ceil(base->Properties().w / 16.0f)),
				static_cast<u32>(glm::ceil(base->Properties().h / 16.0f))
			},
			GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
		);
	}

	void FractalNoise3D::Fill(AssetHandle<Texture3D> textureA, AssetHandle<Texture3D> baseNoise, const FractalNoiseParams& fParams)
	{
		static AssetHandle<ComputeShader> shaderHandle = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "FRACTAL3D");

		auto [shader, g] = AssetManager::GetAsset(shaderHandle);
		auto [texture, g1] = AssetManager::GetAsset(textureA);
		auto [base, g2] = AssetManager::GetAsset(baseNoise);

		if (!shader)
		{
			spdlog::error("Shader for perlin noise is not loaded.");
			return;
		}

		if (!texture || !base)
		{
			spdlog::error("Texture for perlin noise is not loaded.");
			return;
		}

		shader->Use();

		shader->BindTextureUnit(*base, 0);
		shader->BindWriteTexture(*texture, 1);

		shader->SetUniform("w", base->Properties().w);
		shader->SetUniform("h", base->Properties().h);
		shader->SetUniform("d", base->Properties().d);
		shader->SetUniform("N", fParams.NOctaves);

		shader->SetUniform("channelFrom", std::clamp(fParams.ChannelFrom, 1u, 4u));
		shader->SetUniform("channelTo", std::clamp(fParams.ChannelTo, 1u, 4u));

		shader->SetUniform("persistence", fParams.Persistence);
		shader->SetUniform("lacunarity", fParams.Lacunarity);

		shader->DispatchWait(
			{
				static_cast<u32>(glm::ceil(base->Properties().w / 8.0f)),
				static_cast<u32>(glm::ceil(base->Properties().h / 8.0f)),
				static_cast<u32>(glm::ceil(base->Properties().d / 8.0f))
			},
			GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
		);
	}
}