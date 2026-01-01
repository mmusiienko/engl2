#include "Fractal.h"
#include "../../core/Global.h"


namespace EnGl
{
	FractalNoise2D::FractalNoise2D(const FractalNoiseParams& fParams, const Texture2D& baseNoise) 
	: Texture2D(
		Texture::CreationInfoFromData{
			.CpuFormat = GL_RGBA,
			.GpuFormat = GL_RGBA32F
		},
		baseNoise.Properties().w, baseNoise.Properties().h
	)
	{
		Fill(fParams, baseNoise);
	}

	void FractalNoise2D::Fill(const FractalNoiseParams& fParams, const Texture2D& baseNoise)
	{
		ref<ComputeShader> shader = Global::ResourceManager->GetOrLoadComputeShader("FRACTAL2D");

		shader->BindTextureUnit(baseNoise, 0);
		shader->BindWriteTexture(*this, 1);

		shader->Use();

		shader->SetUniform("w", baseNoise.Properties().w);
		shader->SetUniform("h", baseNoise.Properties().h);

		shader->SetUniform("channelFrom", std::clamp(fParams.ChannelFrom, 1u, 4u));
		shader->SetUniform("channelTo", std::clamp(fParams.ChannelTo, 1u, 4u));

		shader->SetUniform("N", fParams.NOctaves);
		shader->SetUniform("persistence", fParams.persistence);
		shader->SetUniform("lacunarity", fParams.lacunarity);

		shader->DispatchWait(
			{
				static_cast<u32>(glm::ceil(baseNoise.Properties().w / 16.0f)),
				static_cast<u32>(glm::ceil(baseNoise.Properties().h / 16.0f))
			},
			GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
		);
	}


	FractalNoise3D::FractalNoise3D(const FractalNoiseParams& fParams, const Texture3D& baseNoise) : Texture3D(
		Texture::CreationInfoFromData{
			.CpuFormat = GL_RGBA,
			.GpuFormat = GL_RGBA32F
		},
		baseNoise.Properties().w, baseNoise.Properties().h, baseNoise.Properties().d
	)
	{
		Fill(fParams, baseNoise);
	}

	void FractalNoise3D::Fill(const FractalNoiseParams& fParams, const Texture3D& baseNoise)
	{
		ref<ComputeShader> shader = Global::ResourceManager->GetOrLoadComputeShader("FRACTAL3D");

		shader->Use();

		shader->BindTextureUnit(baseNoise, 0);
		shader->BindWriteTexture(*this, 1);

		shader->SetUniform("w", baseNoise.Properties().w);
		shader->SetUniform("h", baseNoise.Properties().h);
		shader->SetUniform("d", baseNoise.Properties().d);
		shader->SetUniform("N", fParams.NOctaves);

		shader->SetUniform("channelFrom", std::clamp(fParams.ChannelFrom, 1u, 4u));
		shader->SetUniform("channelTo", std::clamp(fParams.ChannelTo, 1u, 4u));

		shader->SetUniform("persistence", fParams.persistence);
		shader->SetUniform("lacunarity", fParams.lacunarity);

		shader->DispatchWait(
			{
				static_cast<u32>(glm::ceil(baseNoise.Properties().w / 8.0f)),
				static_cast<u32>(glm::ceil(baseNoise.Properties().h / 8.0f)),
				static_cast<u32>(glm::ceil(baseNoise.Properties().d / 8.0f))
			},
			GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
		);
	}
}