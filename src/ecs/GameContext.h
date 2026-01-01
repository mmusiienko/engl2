#pragma once
#include "components/Components.h"
#include "../resources/DebugMesh.h"
#include "../renderer/base/Mesh.h"


namespace EnGl
{
	struct GameContext
	{
		struct CameraInfo
		{
			struct Camera
			{
				EcsImpl::Entity Entity;
				bool CanRotate = false;
				glm::vec3* Position;
				glm::vec3 Forward;
				glm::vec3 Right;
				glm::vec3 Up;
				glm::mat4* View;
				glm::mat4* Projection;
				glm::mat4 InverseView;
				glm::mat4 InverseProjection;
				glm::mat4 ViewProjection;
				glm::mat4 InverseViewProjection;

				f32 Near;
				f32 Far;
				f32 Aspect;
				f32 FovDegree;
			};


			std::vector<Camera> Cameras;
			size_t CameraIdx = 0;
			
			inline Camera Get() const { return Cameras[CameraIdx]; }
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

			struct Hash
			{
				size_t operator()(const InstancedMaterialMapKey& key) const
				{
					size_t res = 0;
					hash_combine(res, key.MeshHandle.Id);
					hash_combine(res, key.MaterialHandle.Id);
					return res;
				}
			};
		};

		struct MaterialMapValue
		{
			struct El
			{
				AssetHandle<Mesh> Mesh;
				Mesh::InstanceData Data;
			};

			std::vector<El> InstanceDatas;
		};

		struct InstancedMaterialMapValue
		{
			std::vector<Mesh::InstanceData> Data;
		};

		using MaterialMap = std::unordered_map<AssetHandle<scope<Material::Base>>, MaterialMapValue>;
		using InstancedMaterialMap = std::unordered_map<InstancedMaterialMapKey, InstancedMaterialMapValue, InstancedMaterialMapKey::Hash>;

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

		struct FramebufferInfo
		{
			AssetHandle<Texture2D> ColorTexture{};
			AssetHandle<Texture2D> ColorTextureLastFrame{};
			AssetHandle<Texture2D> DepthTexture{};
			AssetHandle<Texture2D> DepthTextureLastFrame{};
			AssetHandle<Texture2D> DepthTextureOpaque{};
			glm::vec2 Resolution{ 1 };
		};

		FramebufferInfo Framebuffer;
		
		Cubemap* Cubemap = nullptr;

		struct DebugInfo
		{
			DebugMesh DebugMeshes;
			u32 DrawMode = GL_FILL;

			struct Draw
			{
				bool Enabled = false;
				bool AABB = false;
				bool Camera = false;
			};

			Draw Draw;
		};

		DebugInfo Debug;

		std::unordered_map<std::string, EcsImpl::Entity> SpecialEntities;
	};
}