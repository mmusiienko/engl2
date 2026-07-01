#include "ecs/systems/renderer/techniques/SSAO.h"
#include "SSAO.h"
#include "ui/Components.h"
#include "math/Math.h"
#include "algorithm/compute/Blur.h"
#include <random>


namespace EnGl::System
{
	static void FillKernel(SSBO& kernel, u32 size)
	{
		std::uniform_real_distribution<f32> d{ 0.0f, 1.0f };
		std::default_random_engine e;

		std::vector<glm::vec4> kernelData;
		kernelData.reserve(size);

		for (u32 i = 0; i < size; i++)
		{
			f32 ur = d(e);
			f32 t1 = d(e);
			f32 t2 = d(e);

			f32 r = glm::pow(ur, 1.4);

			f32 cosTheta = t1;
			f32 sinTheta = glm::sqrt(1.0f - cosTheta * cosTheta);

			f32 phi = Math::TWO_PI * t2;

			kernelData.emplace_back(
				r * sinTheta * glm::cos(phi),
				r * sinTheta * glm::sin(phi),
				r * cosTheta,
				0.0f
			);
		}
		kernel.Resize(kernelData.data(), size * sizeof(glm::vec4));
	}

	static void FillRotations(SSBO& rotations, u32 size)
	{
		std::uniform_real_distribution<f32> d{ -1.0f, 1.0f };
		std::default_random_engine e;

		std::vector<glm::vec2> rotationData;
		rotationData.reserve(size);

		for (u32 i = 0; i < size; i++)
		{
			f32 u1 = d(e);
			f32 u2 = d(e);

			rotationData.emplace_back(
				u1,
				u2
			);
		}

		rotations.Resize(rotationData.data(), size * sizeof(glm::vec2));
	}


	void SSAO::Init(EcsImpl::EntityManager& manager)
	{
		m_Texture = AssetManager::Put<Texture2D>(1u, 1u, Texture::CreationInfoFromData{ .CpuFormat = GL_RED, .GpuFormat = GL_R8 });
		m_Texture2 = AssetManager::Put<Texture2D>(1u, 1u, Texture::CreationInfoFromData{ .CpuFormat = GL_RED, .GpuFormat = GL_R8 });
		FillKernel(m_Kernel, m_KernelSize);
		FillRotations(m_Rotations, m_RotationsSqrt * m_RotationsSqrt);
	}

	void SSAO::Run(EcsImpl::EntityManager& manager, GameContext& context)
	{
		static AssetHandle<ComputeShader> shaderHandle = AssetManager::Load<ComputeShader>(AssetManager::COMPUTE_SHADER_DIR / "SSAO");

		auto shader = AssetManager::GetAsset(shaderHandle).Asset;

		if (!shader) return;

		auto texture = AssetManager::GetAsset(m_Texture).Asset;
		auto texture2 = AssetManager::GetAsset(m_Texture2).Asset;
		if (!texture || !texture2) return;

		if (context.Framebuffer.MainFramebuffer->Resolution() != m_Res)
		{
			m_Res = context.Framebuffer.MainFramebuffer->Resolution();
			texture->Properties().w = m_Res.x;
			texture->Properties().h = m_Res.y;
			texture->Update();

			texture2->Properties().w = m_Res.x;
			texture2->Properties().h = m_Res.y;
			texture2->Update();
		}

		auto color = AssetManager::GetAsset(context.Framebuffer.MainFramebuffer->Color()[0]).Asset;
		auto normals = AssetManager::GetAsset(context.Framebuffer.MainFramebuffer->Color()[1]).Asset;
		auto depth = AssetManager::GetAsset(context.Renderer.DepthWithoutTransparents).Asset;
		if (!color || !normals || !depth) return;

		shader->Use();
		shader->BindWriteTexture(*texture, 0);
		shader->SetUniform("uNormals", *normals, 1);
		shader->SetUniform("uDepth", *depth, 2);
		shader->SetUniform("uNear", context.Camera.Get().Near);
		shader->SetUniform("uProjection", context.Camera.Get().Projection);
		shader->SetUniform("uInvProjection", context.Camera.Get().InverseProjection);
		shader->SetUniform("uBias", m_Bias);
		shader->SetUniform("uRadius", m_Radius);
		shader->SetUniform("uNumSamples", m_KernelSize);
		shader->SetUniform("uNumRotationsSqrt", m_RotationsSqrt);
		glm::mat3 v = glm::transpose(glm::inverse(glm::mat3{ context.Camera.Get().View }));
		shader->SetUniform("uViewN", v);
		m_Kernel.Bind(0);
		m_Rotations.Bind(1);

		shader->DispatchWait(
			ComputeShader::ComputeInfo{ .GroupSizeX = static_cast<u32> (glm::ceil(m_Res.x / 16.0f)), .GroupSizeY = static_cast<u32> (glm::ceil(m_Res.y / 16.0f)) },
			GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT
		);

		Compute::Blur(texture, texture2, {.Depth = depth, .NearPlane = context.Camera.Get().Near, .DepthReject = m_DepthRejectBlur});

		context.Renderer.SSAO = m_Texture;
	}

	void SSAO::Editor(EcsImpl::EntityManager& manager, GameContext& context)
	{
		UiComponents::Texture2DView(m_Texture, glm::vec2{ 0.1f });
		UiComponents::Texture2DView(context.Framebuffer.MainFramebuffer->Color()[0], glm::vec2{ 0.1f });
		UiComponents::Texture2DView(context.Framebuffer.MainFramebuffer->Color()[1], glm::vec2{ 0.1f });
		UiComponents::Texture2DView(context.Renderer.DepthWithoutTransparents, glm::vec2{ 0.1f });

		ImGui::InputFloat("Bias", &m_Bias);
		ImGui::InputFloat("Radius", &m_Radius);
		ImGui::InputFloat("DepthRejectBlur", &m_DepthRejectBlur);

		if (UiComponents::InputUInt("KernelSize", &m_KernelSize))
			FillKernel(m_Kernel, m_KernelSize);

		if (UiComponents::InputUInt("RotationSqrt", &m_RotationsSqrt))
			FillRotations(m_Rotations, m_RotationsSqrt * m_RotationsSqrt);
	}
}


