#pragma once

#include "components/Components.h"
#include "resources/DebugMesh.h"
#include "renderer/base/Mesh.h"
#include "renderer/base/Framebuffer.h"


namespace EnGl
{
	struct GameContext
	{
		struct CameraInfo
		{
			struct Camera
			{
				Entity Entity;
				bool CanRotate = false;
				glm::vec3* Position;
				glm::vec3 Forward;
				glm::vec3 Right;
				glm::vec3 Up;
				glm::vec3 Delta;

				glm::mat4* View;
				glm::mat4 ViewLastFrame;
				glm::mat4 InverseView;
				glm::mat4 InverseViewLastFrame;

				glm::mat4* Projection;
				glm::mat4 ProjectionLastFrame;
				glm::mat4 InverseProjection;
				glm::mat4 InverseProjectionLastFrame;

				glm::mat4 ViewProjection;
				glm::mat4 ViewProjectionLastFrame;
				glm::mat4 InverseViewProjection;
				glm::mat4 InverseViewProjectionLastFrame;

				f32 Near;
				f32 Far;
			};

			std::vector<Camera> Cameras;
			u32 CameraIdx = 0;

			inline Camera& Get() { return Cameras[CameraIdx]; }
			inline const Camera& Get() const { return Cameras[CameraIdx]; }
		};

		struct InstancedMaterialMapKey
		{
			AssetHandle<Mesh> MeshHandle;
			AssetHandle<Material::Base> MaterialHandle;

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
				AssetHandle<Material::Base> Material;
				Mesh::InstanceData Data;
			};

			std::vector<El> InstanceDatas;
		};

		struct InstancedMaterialMapValue
		{
			std::vector<Mesh::InstanceData> Data;
		};

		using MaterialMap = std::unordered_map<u32, MaterialMapValue>;
		using InstancedMaterialMap = std::unordered_map<InstancedMaterialMapKey, InstancedMaterialMapValue, InstancedMaterialMapKey::Hash>;

		struct RendererInfo
		{
			std::array<MaterialMap, Component::RenderLayerNumber> PerMaterial;
			std::array<InstancedMaterialMap, Component::RenderLayerNumber> PerInstancedMaterial;
			AssetHandle<Material::Base> MaterialOverride;
			struct CascadedShadowMapInfo
			{


				static constexpr u32 NShadowMapCascades = 3u;

				Entity CascadeCamera[NShadowMapCascades];
				Framebuffer* DirShadowFramebuffer[NShadowMapCascades];
				u32 TextureSize[NShadowMapCascades]{ 2048u, 2048u, 4096u };
				f32 DepthSplit[NShadowMapCascades]{ 0.883f, 0.994f, 1.0f };
			} CascadedShadowMap;
		};

		f64 Time = 0.0f;
		f32 DeltaTime = 0.0f;

		glm::vec2 MouseDelta;

		CameraInfo Camera;

		RendererInfo Renderer;

		struct FramebufferInfo
		{
			AssetHandle<Texture2D> DepthTextureOpaque{};
			Framebuffer* MainFramebuffer = nullptr;
		};

		FramebufferInfo Framebuffer;

		struct CubemapInfo
		{
			AssetHandle<Cubemap> Asset{};
			Entity Id;
			bool Dirty = false;
		} Cubemap;

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

		struct DirLightContext
		{
			Shader::UniformDirectionalLight Data;
			glm::quat Rotation;
			Entity Id = 0;
		} DirLight;

		std::array<Shader::UniformPointLight, Shader::MAX_LIGHTS> PointLights;

		AssetHandle<Texture2D> SkyTexture;
		AssetHandle<Texture2D> SkyTextureLowRes;

		std::unordered_map<std::string, Entity> SpecialEntities;
	};
}