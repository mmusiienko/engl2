#pragma once
#include "../resources/importers/AssetManager.h"


namespace EnGl
{
	struct NoiseParams
	{
		u32 NCells = 8u;
		u32 NChannel = 0u;
		u32 Seed = 0u;
		u32 Octaves = 1u;
		f32 DarkThreshold = 0.0f;
	};

	struct Noise2D
	{
	public:
		static void Perlin(AssetHandle<Texture2D> textureA, const NoiseParams& params)
		{
			static const AssetHandle<ComputeShader> shaderHandle = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "FBMPerlin2D");
			Fill(shaderHandle, textureA, params);
		}

		static void Worley(AssetHandle<Texture2D> textureA, const NoiseParams& params)
		{
			static const AssetHandle<ComputeShader> shaderHandle = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "FBMWorley2D");
			Fill(shaderHandle, textureA, params);
		}

	private:
		static void Fill(AssetHandle<ComputeShader> shaderA, AssetHandle<Texture2D> textureA, const NoiseParams& params)
		{
			auto shader = AssetManager::GetAsset(shaderA).Asset;
			auto texture = AssetManager::GetAsset(textureA).Asset;

			if (!shader)
			{
				spdlog::error("Shader for noise is not loaded.");
				return;
			}

			if (!texture)
			{
				spdlog::error("Texture for noise is not loaded.");
				return;
			}

			shader->Use();
			shader->SetUniform("uNCells", params.NCells);
			shader->SetUniform("uChannel", std::clamp(params.NChannel, 0u, 3u));
			shader->SetUniform("uSeed", params.Seed);
			shader->SetUniform("uOctaves", params.Octaves);
			shader->SetUniform("uDarkThreshold", params.DarkThreshold);

			shader->BindReadWriteTexture(*texture, 0);

			shader->DispatchWait(
				{
					static_cast<u32>(glm::ceil(texture->Properties().w / 16.0f)),
					static_cast<u32>(glm::ceil(texture->Properties().h / 16.0f))
				},
				GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
			);
		}
	};

	struct Noise3D
	{
	public:
		static void Perlin(AssetHandle<Texture3D> textureA, const NoiseParams& params)
		{
			static const AssetHandle<ComputeShader> shaderHandle = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "FBMPerlin3D");
			Fill(shaderHandle, textureA, params);
		}

		static void Worley(AssetHandle<Texture3D> textureA, const NoiseParams& params)
		{
			static const AssetHandle<ComputeShader> shaderHandle = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "FBMWorley3D");
			Fill(shaderHandle, textureA, params);
		}

		static void PerlinWorley(AssetHandle<Texture3D> textureA, const NoiseParams& params)
		{
			static const AssetHandle<ComputeShader> shaderHandle = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "FBMPerlinWorley3D");
			Fill(shaderHandle, textureA, params);
		}

	private:
		static void Fill(AssetHandle<ComputeShader> shaderA, AssetHandle<Texture3D> textureA, const NoiseParams& params)
		{
			auto shader = AssetManager::GetAsset(shaderA).Asset;
			auto texture = AssetManager::GetAsset(textureA).Asset;

			if (!shader)
			{
				spdlog::error("Shader for noise is not loaded.");
				return;
			}

			if (!texture)
			{
				spdlog::error("Texture for noise is not loaded.");
				return;
			}

			shader->Use();
			shader->SetUniform("uNCells", params.NCells);
			shader->SetUniform("uChannel", std::clamp(params.NChannel, 0u, 3u));
			shader->SetUniform("uSeed", params.Seed);
			shader->SetUniform("uOctaves", params.Octaves);

			shader->BindReadWriteTexture(*texture, 0);

			shader->DispatchWait(
				{
					static_cast<u32>(glm::ceil(texture->Properties().w / 8.0f)),
					static_cast<u32>(glm::ceil(texture->Properties().h / 8.0f)),
					static_cast<u32>(glm::ceil(texture->Properties().d / 8.0f))
				},
				GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
			);
		}
	};
}