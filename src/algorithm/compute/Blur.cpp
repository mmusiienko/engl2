#include "Blur.h"


namespace EnGl::Compute
{
	void Blur(Texture2D* src, Texture2D* dst, DepthAware depthAware)
	{
		static const std::unordered_map<u32, std::pair<AssetHandle<ComputeShader>, AssetHandle<ComputeShader>>> FORMAT_TO_SHADER
		{
			{
				GL_R8,
				{
					AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "BLUR" / "BlurVr8"),
					AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "BLUR" / "BlurHr8")
				}
			}
		};


		auto format = src->Properties().GpuFormat;
		auto res = glm::uvec2{ src->Properties().w, src->Properties().h };
		if (!FORMAT_TO_SHADER.contains(format)) return;

		auto [vH, hH] = FORMAT_TO_SHADER.at(format);

		auto shaderV = AssetManager::GetAsset(vH).Asset;
		auto shaderH = AssetManager::GetAsset(hH).Asset;

		if (!shaderV || !shaderH) return;

		shaderV->Use();

		shaderV->BindReadTexture(*src, 0);
		shaderV->BindWriteTexture(*dst, 1);

		if (depthAware.Depth)
		{
			shaderH->SetUniform("uDepth", *depthAware.Depth, 2);
			shaderH->SetUniform("uDepthReject", depthAware.DepthReject);
			shaderH->SetUniform("uNear", depthAware.NearPlane);
			shaderH->SetUniform("uDepthAware", 1u);
		}
		else
		{
			shaderH->SetUniform("uDepthAware", 0u);
		}

		shaderV->DispatchWait(
			{
				.GroupSizeX = static_cast<u32> (glm::ceil(res.x / 16.0f)),
				.GroupSizeY = static_cast<u32> (glm::ceil(res.y / 16.0f))
			}
			, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT
		);

		shaderH->Use();

		shaderH->BindReadTexture(*dst, 0);
		shaderH->BindWriteTexture(*src, 1);

		if (depthAware.Depth)
		{
			shaderH->SetUniform("uDepth", *depthAware.Depth, 2);
			shaderH->SetUniform("uDepthReject", depthAware.DepthReject);
			shaderH->SetUniform("uNear", depthAware.NearPlane);
			shaderH->SetUniform("uDepthAware", 1u);
		}
		else
		{
			shaderH->SetUniform("uDepthAware", 0u);
		}

		shaderH->DispatchWait(
			{
				.GroupSizeX = static_cast<u32> (glm::ceil(res.x / 16.0f)),
				.GroupSizeY = static_cast<u32> (glm::ceil(res.y / 16.0f))
			}
			, GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT
		);
	}
}

