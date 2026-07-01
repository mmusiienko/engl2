#pragma once

#include "resources/importers/AssetManager.h"
#include "renderer/base/Texture.h"
#include "renderer/base/ComputeShader.h"


namespace EnGl
{
	namespace Compute
	{
		struct DepthAware
		{
			Texture2D* Depth = nullptr;
			f32 NearPlane = 0.1f;
			f32 DepthReject = 0.2f;
		};

		void Blur(Texture2D* source, Texture2D* dest, DepthAware depthAware = {});
	};
}