#include "Perlin.h"
#include "../math/Random.h"
#include "../core/Global.h"
#include "../resources/renderer/base/SSBO.h"
#include "spdlog/spdlog.h"


namespace EnGl
{
	PerlinNoise2D::PerlinNoise2D(const PerlinNoise2DParams& params) : Texture2D(
		Texture::CreationInfoFromData{
			.CpuFormat = GL_RGBA,
			.GpuFormat = GL_RGBA32F
		},
		params.w, params.h
	)
	{
		Fill(params);
	}

	void PerlinNoise2D::Fill(const PerlinNoise2DParams& params)
	{
		auto gradients = GenerateGradientsOnVertices(params.NCells);

		RunCompute(gradients, params);
	}

	void PerlinNoise2D::RunCompute(const std::vector<glm::vec2>& gradients, const PerlinNoise2DParams& params)
	{
		ref<ComputeShader> shader = Global::ResourceManager->GetOrLoadComputeShader("PERLIN2D");

		shader->Use();
		shader->SetUniform("NCells", params.NCells);
		shader->SetUniform("w", params.w);
		shader->SetUniform("h", params.h);
		shader->SetUniform("channel", std::clamp(params.NChannel, 1u, 4u));
		shader->SetUniform("cellW", static_cast<f32>(params.w) / params.NCells);
		shader->SetUniform("cellH", static_cast<f32>(params.h) / params.NCells);

		SSBO ssbo {gradients.data(), gradients.size() * sizeof(glm::vec2)};

		shader->BindSSBO(ssbo, 0);
		shader->BindReadWriteTexture(*this, 1);
		shader->DispatchWait(
			{ 
				static_cast<u32>(glm::ceil(params.w / 16.0f)),
				static_cast<u32>(glm::ceil(params.h / 16.0f))
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

	PerlinNoise3D::PerlinNoise3D(const PerlinNoise3DParams& params) : Texture3D(
		Texture::CreationInfoFromData{
			.CpuFormat = GL_RGBA,
			.GpuFormat = GL_RGBA32F
		},
		params.w, params.h, params.d
	)
	{
		Fill(params);
	}

	void PerlinNoise3D::Fill(const PerlinNoise3DParams& params)
	{
		auto gradients = GenerateGradientsOnVertices(params.NCells);

		RunCompute(gradients, params);
	}

	void PerlinNoise3D::RunCompute(const std::vector<glm::vec4>& gradients, const PerlinNoise3DParams& params)
	{
		ref<ComputeShader> shader = Global::ResourceManager->GetOrLoadComputeShader("PERLIN3D");

		shader->Use();
		shader->SetUniform("NCells", params.NCells);
		shader->SetUniform("w", params.w);
		shader->SetUniform("h", params.h);
		shader->SetUniform("d", params.d);
		shader->SetUniform("channel", std::clamp(params.NChannel, 1u, 4u));
		shader->SetUniform("cellW", static_cast<f32>(params.w) / params.NCells);
		shader->SetUniform("cellH", static_cast<f32>(params.h) / params.NCells);
		shader->SetUniform("cellD", static_cast<f32>(params.d) / params.NCells);

		SSBO ssbo{ gradients.data(), gradients.size() * sizeof(glm::vec4) };

		shader->BindSSBO(ssbo, 0);
		shader->BindReadWriteTexture(*this, 1);

		shader->DispatchWait(
			{
				static_cast<u32>(glm::ceil(params.w / 8.0f)),
				static_cast<u32>(glm::ceil(params.h / 8.0f)),
				static_cast<u32>(glm::ceil(params.d / 8.0f))
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


