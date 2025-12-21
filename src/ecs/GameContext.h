#pragma once
#include "components/Components.h"


namespace EnGl
{
	struct GameContext
	{
		struct CameraInfo
		{
			std::vector<EcsImpl::Entity> Cameras;
			size_t CameraIdx = 0;
			glm::mat4* View;
			glm::mat4* Projection;

			inline EcsImpl::Entity Get() const { return Cameras[CameraIdx]; }
		};

		struct InstancedMaterialMapKey
		{
			AssetHandle<Mesh> MeshHandle;
			AssetHandle<scope<Material::Base>> MaterialHandle;

			bool operator==(const InstancedMaterialMapKey& other) const
			{
				return MeshHandle.Id == other.MeshHandle.Id &&
					MaterialHandle.Id == other.MaterialHandle.Id;
			}

			size_t operator()(const InstancedMaterialMapKey& key) const
			{
				size_t res = 0;
				hash_combine(res, key.MeshHandle.Id);
				hash_combine(res, key.MaterialHandle.Id);
				return res;
			}
		};

		struct MaterialMapValue
		{
			struct El
			{
				AssetHandle<Mesh> Mesh;
				glm::mat4 Model;
			};

			std::vector<El> Transforms;
		};

		struct InstancedMaterialMapValue
		{
			std::vector<glm::mat4> Transforms;
		};

		using MaterialMap = std::unordered_map<AssetHandle<scope<Material::Base>>, MaterialMapValue>;
		using InstancedMaterialMap = std::unordered_map<InstancedMaterialMapKey, InstancedMaterialMapValue, InstancedMaterialMapKey>;

		struct RendererInfo
		{
			std::array<MaterialMap, Component::RenderLayerNumber> PerMaterial;
			std::array<InstancedMaterialMap, Component::RenderLayerNumber> PerInstancedMaterial;
		};

		f64 Time = 0.0f;
		f32 DeltaTime = 0.0f;

		glm::vec2 MouseDelta;

		CameraInfo Camera;

		RendererInfo Renderer;

		Texture2D* ColorTexture = nullptr;
		Texture2D* DepthTexture = nullptr;
	};
}