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
		};

		struct CreationInfo
		{
			std::vector<Vertex> Vertices;
			std::vector<Index> Indices;
			bool HasNormals;
			bool HasTextureCoords;
		};

		Mesh(const CreationInfo& info);

		Mesh(Mesh&& other) noexcept
		{
			m_Id = other.m_Id;
			m_VBO = other.m_VBO;
			m_EBO = other.m_EBO;
			m_SSBO = std::move(other.m_SSBO);
			m_IndicesSize = other.m_IndicesSize;
			m_InstanceSize = other.m_InstanceSize;
			other.m_Id = 0;
			other.m_VBO = 0;
			other.m_EBO = 0;
			other.m_IndicesSize = 0;
			other.m_InstanceSize = 0;
		};

		Mesh& operator=(Mesh&& other) noexcept
		{
			std::swap(m_Id, other.m_Id);
			std::swap(m_VBO, other.m_VBO);
			std::swap(m_EBO, other.m_EBO);
			m_SSBO = std::move(other.m_SSBO);
			m_IndicesSize = other.m_IndicesSize;
			m_InstanceSize = other.m_InstanceSize;
			other.m_IndicesSize = 0;
			other.m_InstanceSize = 0;

			return *this;
		};

		void Draw() const;
		void UpdateInstanceBuffer(const std::vector<glm::mat4>& transforms);
		void DrawInstanced();
		~Mesh() override;
		static void Unbind();
	private:
		size_t m_IndicesSize = 0;
		u32 m_VBO = 0;
		u32 m_EBO = 0;

		SSBO m_SSBO{ nullptr, 0 };
		size_t m_InstanceSize = 0;
	};
}