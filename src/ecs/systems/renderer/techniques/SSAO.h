#pragma once
#include "ecs/systems/Systems.h"


namespace EnGl::System
{
	class SSAO : public SystemImpl
	{
		void Init(EcsImpl::EntityManager& manager) override;
		void Run(EcsImpl::EntityManager& manager, GameContext& context) override;
		void Editor(EcsImpl::EntityManager& manager, GameContext& context) override;

		AssetHandle<Texture2D> m_Texture;
		AssetHandle<Texture2D> m_Texture2;
		glm::uvec2 m_Res{ 1u };

		f32 m_Bias = 0.05f;
		f32 m_Radius = 2.0f;
		f32 m_DepthRejectBlur = 0.2f;

		u32 m_KernelSize = 16u;
		u32 m_RotationsSqrt = 4u;

		SSBO m_Kernel{ nullptr, 0zu };
		SSBO m_Rotations{ nullptr, 0zu };
	};
}
