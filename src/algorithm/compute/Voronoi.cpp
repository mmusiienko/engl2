#include "Voronoi.h"
#include "../math/Random.h"
#include "../core/Global.h"


namespace EnGl
{
	VoronoiNoise2D::VoronoiNoise2D(const VoronoiNoise2DParams& params) : Texture2D(
		Texture::CreationInfoFromData{
			.CpuFormat = GL_RED,
			.GpuFormat = GL_R32F
		},
		params.w, params.h
	)
	{ 
		Fill(params);
	}

	void VoronoiNoise2D::Fill(const VoronoiNoise2DParams& params)
	{
		auto points = GeneratePointsWithinBounds(params);

		RunCompute(points, params);
	}

	void VoronoiNoise2D::RunCompute(const std::vector<glm::vec2>& points, const VoronoiNoise2DParams& params)
	{
		Texture2D pointsTex{ {.Data = (void*)points.data(), .CpuFormat = GL_RG, .GpuFormat = GL_RG32F}, static_cast<u32>(points.size()), 1 };

		f32 cellW = static_cast<f32>(params.w) / params.NPoints;
		f32 cellH = static_cast<f32>(params.h) / params.NPoints;
		f32 maxDist = cellW * cellW + cellH * cellH;

		ref<ComputeShader> shader = Global::ResourceManager->GetOrLoadComputeShader("VORONOI2D");

		shader->Use();
		shader->SetUniform("NPoints", params.NPoints);
		shader->SetUniform("maxDist", maxDist);
		shader->SetUniform("w", params.w);
		shader->SetUniform("h", params.h);
		shader->SetUniform("channel", std::clamp(params.NChannel, 1u, 4u));

		shader->BindReadTexture(pointsTex, 0);
		shader->BindReadWriteTexture(*this, 1);

		shader->DispatchWait(
			{
				static_cast<u32>(glm::ceil(params.w / 16.0f)),
				static_cast<u32>(glm::ceil(params.h / 16.0f))
			},
			GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
		);
	}

	std::vector<glm::vec2> VoronoiNoise2D::GeneratePointsWithinBounds(const VoronoiNoise2DParams& params)
	{
		auto idx = [params](u32 i, u32 j) { return i * (params.NPoints + 2) + j; };

		std::vector<glm::vec2> res((params.NPoints + 2) * (params.NPoints + 2));
		Random rand;
		f32 cellW = static_cast<f32>(params.w) / params.NPoints;
		f32 cellH = static_cast<f32>(params.h) / params.NPoints;

		for (u32 i = 1; i < params.NPoints + 1; i++)
		{
			for (u32 j = 1; j < params.NPoints + 1; j++)
			{
				auto id = idx(i, j);
				auto randid = 2 * id;

				auto first = glm::floor(rand.UniformRange(cellW * (i - 1), cellW * i));
				rand.AddToState(randid);
				auto second = glm::floor(rand.UniformRange(cellH * (j - 1), cellH * j));
				rand.AddToState(randid + 1);

				res[id] = {first, second};
			}
		}
		
		auto wrap = [params](u32 i) { return (i - 1 + params.NPoints) % params.NPoints + 1; };

		for (u32 i = 0; i <= params.NPoints + 1; i++)
		{
			for (u32 j = 0; j <= params.NPoints + 1; j++)
			{
				if (i >= 1 && i <= params.NPoints && j >= 1 && j <= params.NPoints) continue;
				f32 dx = 0, dy = 0;
				if (i == 0) dx = -static_cast<f32>(params.w);
				if (i == params.NPoints + 1) dx = static_cast<f32>(params.w);
				if (j == 0) dy = -static_cast<f32>(params.h);
				if (j == params.NPoints + 1) dy = static_cast<f32>(params.h);
				res[idx(i, j)] = res[idx(wrap(i), wrap(j))] + glm::vec2(dx, dy);
			}
		}

		return res;
	}

	VoronoiNoise3D::VoronoiNoise3D(const VoronoiNoise3DParams& params) : Texture3D(
		Texture::CreationInfoFromData{
			.CpuFormat = GL_RGBA,
			.GpuFormat = GL_RGBA32F
		},
		params.w, params.h, params.d
	)
	{
		Fill(params);
	}

	void VoronoiNoise3D::Fill(const VoronoiNoise3DParams& params)
	{
		auto points = GeneratePointsWithinBounds(params);

		RunCompute(points, params);
	}

	void VoronoiNoise3D::RunCompute(const std::vector<glm::vec4>& points, const VoronoiNoise3DParams& params)
	{
		SSBO pointsSSBO{ points.data(), points.size() * sizeof(glm::vec4) };

		f32 cellW = static_cast<f32>(params.w) / params.NPoints;
		f32 cellH = static_cast<f32>(params.h) / params.NPoints;
		f32 cellD = static_cast<f32>(params.d) / params.NPoints;
		f32 maxDist = cellW * cellW + cellH * cellH + cellD * cellD;

		ref<ComputeShader> shader = Global::ResourceManager->GetOrLoadComputeShader("VORONOI3D");

		shader->Use();
		shader->SetUniform("NPoints", params.NPoints);
		shader->SetUniform("maxDistSquared", maxDist);
		shader->SetUniform("w", params.w);
		shader->SetUniform("h", params.h);
		shader->SetUniform("d", params.d);
		shader->SetUniform("channel", std::clamp(params.NChannel, 1u, 4u));

		shader->BindSSBO(pointsSSBO, 0);
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

	std::vector<glm::vec4> VoronoiNoise3D::GeneratePointsWithinBounds(const VoronoiNoise3DParams& params)
	{
		auto idx = [params](u32 i, u32 j, u32 k) { return i * (params.NPoints + 2) * (params.NPoints + 2) + j * (params.NPoints + 2) + k; };
		
		std::vector<glm::vec4> res((params.NPoints + 2) * (params.NPoints + 2) * (params.NPoints + 2));
		Random rand;
		f32 cellW = static_cast<f32>(params.w) / params.NPoints;
		f32 cellH = static_cast<f32>(params.h) / params.NPoints;
		f32 cellD = static_cast<f32>(params.d) / params.NPoints;

		for (u32 i = 1; i < params.NPoints + 1; i++)
		{
			for (u32 j = 1; j < params.NPoints + 1; j++)
			{
				for (u32 k = 1; k < params.NPoints + 1; k++)
				{
					auto id = idx(i, j, k);
					auto randid = 3 * id;

					auto first = glm::floor(rand.UniformRange(cellW * (i - 1), cellW * i));
					rand.AddToState(randid);
					auto second = glm::floor(rand.UniformRange(cellH * (j - 1), cellH * j));
					rand.AddToState(randid + 1);
					auto third = glm::floor(rand.UniformRange(cellD * (k - 1), cellD * k));
					rand.AddToState(randid + 2);
					res[id] = { first, second, third, 0 };
				}
			}
		}

		auto wrap = [params](u32 i) { return (i - 1 + params.NPoints) % params.NPoints + 1; };

		for (u32 i = 0; i <= params.NPoints + 1; i++)
		{
			for (u32 j = 0; j <= params.NPoints + 1; j++)
			{
				for (u32 k = 0; k <= params.NPoints + 1; k++)
				{
					if (i >= 1 && i <= params.NPoints && j >= 1 && j <= params.NPoints && k >= 1 && k <= params.NPoints) continue;
					f32 dx = 0, dy = 0, dz = 0;

					if (i == 0) dx = -static_cast<f32>(params.w);
					if (i == params.NPoints + 1) dx = static_cast<f32>(params.w);
					if (j == 0) dy = -static_cast<f32>(params.h);
					if (j == params.NPoints + 1) dy = static_cast<f32>(params.h);
					if (k == 0) dz = -static_cast<f32>(params.d);
					if (k == params.NPoints + 1) dz = static_cast<f32>(params.d);
					res[idx(i, j, k)] = res[idx(wrap(i), wrap(j), wrap(k))] + glm::vec4(dx, dy, dz, 0);
				}
			}
		}

		return res;
	}
}