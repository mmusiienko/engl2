#pragma once

#include "../math/Math.h"
#include <vector>
#include "Resource.h"
#include "SSBO.h"


namespace EnGl
{
	class Mesh : public Resource
	{
	public:
		typedef u32 Index;

		struct Vertex
		{
			glm::vec3 Position;
			glm::vec3 Normal;
			glm::vec2 TexCoord;
			glm::vec3 Tangent;
			glm::vec3 BiTangent;
		};

		struct AABB
		{
			glm::vec3 Min{};
			glm::vec3 Max{};
		};

		struct CreationInfo
		{
			std::vector<Vertex> Vertices;
			std::vector<Index> Indices;
			bool HasNormals = false;
			bool HasTextureCoords = false;
			bool HasTangents = false;
			AABB Aabb{};
			u32 DrawType = GL_TRIANGLES;
		};

		Mesh(const CreationInfo& info);

		Mesh(Mesh&& other) noexcept
		{
			m_Id = other.m_Id;
			m_VBO = other.m_VBO;
			m_EBO = other.m_EBO;
			m_InstanceData = std::move(other.m_InstanceData);
			m_IndicesSize = other.m_IndicesSize;
			m_InstanceSize = other.m_InstanceSize;
			m_AABB = other.m_AABB;
			m_DrawType = other.m_DrawType;
			other.m_Id = 0;
			other.m_VBO = 0;
			other.m_EBO = 0;
			other.m_IndicesSize = 0;
			other.m_InstanceSize = 0;
			other.m_AABB = {};
			other.m_DrawType = GL_TRIANGLES;
		};

		Mesh& operator=(Mesh&& other) noexcept
		{
			std::swap(m_Id, other.m_Id);
			std::swap(m_VBO, other.m_VBO);
			std::swap(m_EBO, other.m_EBO);
			std::swap(m_DrawType, other.m_DrawType);
			std::swap(m_IndicesSize, other.m_IndicesSize);
			std::swap(m_InstanceSize, other.m_InstanceSize);
			std::swap(m_AABB, other.m_AABB);
			std::swap(m_InstanceData, other.m_InstanceData);
			return *this;
		};

		void Draw() const;

		struct InstanceData
		{
			glm::mat4 Model;
			glm::mat3x4 Normal;
		};

		void UpdateInstanceBuffer(const std::vector<InstanceData>& instanceData);
		void DrawInstanced();
		~Mesh() override;

		inline AABB& GetAABB() { return m_AABB; }
		inline const AABB& GetAABB() const { return m_AABB; }

		inline u32 DrawType() const { return m_DrawType; }
	private:
		u32 m_DrawType = GL_TRIANGLES;

		size_t m_IndicesSize = 0;
		u32 m_VBO = 0;
		u32 m_EBO = 0;
		AABB m_AABB{};
		SSBO m_InstanceData{ nullptr, 0 };
		size_t m_InstanceSize = 0;
	};
}