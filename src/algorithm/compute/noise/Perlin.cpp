#include "Perlin.h"
#include "../math/Random.h"
#include "../core/Global.h"
#include "../renderer/base/SSBO.h"
#include "spdlog/spdlog.h"


namespace EnGl
{
	void PerlinNoise2D::Fill(AssetHandle<Texture2D> textureA, const PerlinNoiseParams& params)
	{
		static AssetHandle<ComputeShader> shaderHandle = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "PERLIN2D");

		auto [shader, g] = AssetManager::GetAsset(shaderHandle);
		auto [texture, g1] = AssetManager::GetAsset(textureA);

		if (!shader)
		{
			spdlog::error("Shader for perlin noise is not loaded.");
			return;
		}

		if (!texture)
		{
			spdlog::error("Texture for perlin noise is not loaded.");
			return;
		}

		auto gradients = GenerateGradientsOnVertices(params.NCells);

		shader->Use();
		shader->SetUniform("NCells", params.NCells);
		shader->SetUniform("w", texture->Properties().w);
		shader->SetUniform("h", texture->Properties().h);
		shader->SetUniform("channel", std::clamp(params.NChannel, 1u, 4u));
		shader->SetUniform("cellW", static_cast<f32>(texture->Properties().w) / params.NCells);
		shader->SetUniform("cellH", static_cast<f32>(texture->Properties().h) / params.NCells);

		SSBO ssbo {gradients.data(), gradients.size() * sizeof(glm::vec2)};

		shader->BindSSBO(ssbo, 0);
		shader->BindReadWriteTexture(*texture, 1);
		shader->DispatchWait(
			{ 
				static_cast<u32>(glm::ceil(texture->Properties().w / 16.0f)),
				static_cast<u32>(glm::ceil(texture->Properties().h / 16.0f))
			},
			GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
		);
	}

	std::vector<glm::vec2> PerlinNoise2D::GenerateGradientsOnVertices(u32 NCells)
	{
		auto idx = [NCells](u32 i, u32 j) { return i * (NCells + 1) + j; };

		std::vector<glm::vec2> res((NCells + 1) * (NCells + 1));
		Random rand;

		for (u32 i = 1; i < NCells; i++)
		{
			for (u32 j = 1; j < NCells; j++)
			{
				auto id = idx(i, j);

				rand.AddToState(id);

				res[id] = glm::normalize(rand.NonUniformOnS1());
			}
		}

		for (u32 i = 0; i < NCells + 1; i++)
		{
			rand.AddToState(i);
			auto vec = glm::normalize(rand.NonUniformOnS1());
			res[idx(i, 0)] = vec;
			res[idx(i, NCells)] = vec;
		}

		for (u32 j = 1; j < NCells; j++)
		{
			rand.AddToState(j);
			auto vec = glm::normalize(rand.NonUniformOnS1());
			res[idx(0, j)] = vec;
			res[idx(NCells, j)] = vec;
		}

		return res;
	}


	void PerlinNoise3D::Fill(AssetHandle<Texture3D> textureA, const PerlinNoiseParams& params)
	{
		static AssetHandle<ComputeShader> shaderHandle = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "PERLIN3D");

		auto [shader, g] = AssetManager::GetAsset(shaderHandle);
		auto [texture, g1] = AssetManager::GetAsset(textureA);

		if (!shader)
		{
			spdlog::error("Shader for perlin noise is not loaded.");
			return;
		}

		if (!texture)
		{
			spdlog::error("Texture for perlin noise is not loaded.");
			return;
		}

		auto gradients = GenerateGradientsOnVertices(params.NCells);

		shader->Use();
		shader->SetUniform("NCells", params.NCells);
		shader->SetUniform("w", texture->Properties().w);
		shader->SetUniform("h", texture->Properties().h);
		shader->SetUniform("d", texture->Properties().d);
		shader->SetUniform("channel", std::clamp(params.NChannel, 1u, 4u));
		shader->SetUniform("cellW", static_cast<f32>(texture->Properties().w) / params.NCells);
		shader->SetUniform("cellH", static_cast<f32>(texture->Properties().h) / params.NCells);
		shader->SetUniform("cellD", static_cast<f32>(texture->Properties().d) / params.NCells);

		SSBO ssbo{ gradients.data(), gradients.size() * sizeof(glm::vec4) };

		shader->BindSSBO(ssbo, 0);
		shader->BindReadWriteTexture(*texture, 1);

		shader->DispatchWait(
			{
				static_cast<u32>(glm::ceil(texture->Properties().w / 8.0f)),
				static_cast<u32>(glm::ceil(texture->Properties().h / 8.0f)),
				static_cast<u32>(glm::ceil(texture->Properties().d / 8.0f))
			},
			GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
		);
	}

	std::vector<glm::vec4> PerlinNoise3D::GenerateGradientsOnVertices(u32 NCells)
	{
		std::vector<glm::vec4> res((NCells + 1) * (NCells + 1) * (NCells + 1));
		Random rand;

		auto idx = [NCells](u32 i, u32 j, u32 k) { return i * (NCells + 1) * (NCells + 1) + j * (NCells + 1) + k; };

		for (u32 i = 0; i <= NCells; i++)
			for (u32 j = 0; j <= NCells; j++)
				for (u32 k = 0; k <= NCells; k++)
				{
					auto id = idx(i, j, k);
					rand.AddToState(id);
					res[id] = { glm::normalize(rand.NonUniformOnS2()), 0 };
				}

		return res;
	}
}


