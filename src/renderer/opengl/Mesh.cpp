#include "renderer/base/Mesh.h"

#include "core/Core.h"
#include "spdlog/spdlog.h"


namespace EnGl
{
	Mesh::Mesh(const CreationInfo& info) : m_AABB(info.Aabb), m_DrawType(info.DrawType)
	{
		GL_CHECK(glGenVertexArrays(1, &m_Id));
		GL_CHECK(glBindVertexArray(m_Id));

		GL_CHECK(glGenBuffers(1, &m_VBO));
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_VBO));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * info.Vertices.size(), info.Vertices.data(), GL_STATIC_DRAW));

		GL_CHECK(glGenBuffers(1, &m_EBO));

		m_IndicesSize = static_cast<u32>(info.Indices.size());

		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO));
		GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * m_IndicesSize, info.Indices.data(), GL_STATIC_DRAW));

		GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position)));
		GL_CHECK(glEnableVertexAttribArray(0));

		if (info.HasNormals)
		{
			GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal)));
			GL_CHECK(glEnableVertexAttribArray(1));

			if (info.HasTangents)
			{
				GL_CHECK(glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent)));
				GL_CHECK(glEnableVertexAttribArray(3));
				GL_CHECK(glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, BiTangent)));
				GL_CHECK(glEnableVertexAttribArray(4));
			}
		}
		else
		{
			GL_CHECK(glDisableVertexAttribArray(1));
			GL_CHECK(glDisableVertexAttribArray(3));
			GL_CHECK(glDisableVertexAttribArray(4));
		}

		if (info.HasTextureCoords)
		{
			GL_CHECK(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoord)));
			GL_CHECK(glEnableVertexAttribArray(2));
		}
		else
		{
			GL_CHECK(glDisableVertexAttribArray(2));
		}
	}

	void Mesh::Draw() const
	{
		GL_CHECK(glBindVertexArray(m_Id));
		GL_CHECK(glDrawElements(m_DrawType, m_IndicesSize, GL_UNSIGNED_INT, 0));
	}

	void Mesh::UpdateInstanceBuffer(const std::vector<InstanceData>& instanceData)
	{
		m_InstanceSize = static_cast<u32>(instanceData.size());
		m_InstanceData.Resize((void*)instanceData.data(), m_InstanceSize * sizeof(InstanceData));
	}

	void Mesh::DrawInstanced()
	{
		glBindVertexArray(m_Id);
		m_InstanceData.Bind(0);
		glDrawElementsInstanced(m_DrawType, m_IndicesSize, GL_UNSIGNED_INT, 0, m_InstanceSize);
	}

	Mesh::~Mesh()
	{
		GL_CHECK(glDeleteVertexArrays(1, &m_Id));
		GL_CHECK(glDeleteBuffers(1, &m_VBO));
		GL_CHECK(glDeleteBuffers(1, &m_EBO));
	}
}