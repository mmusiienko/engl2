#include <glad/glad.h>

#include "../base/Mesh.h"
#include "../core/Core.h"

namespace EnGl
{
	Mesh::Mesh(const CreationInfo& info)
	{
		GL_CHECK(glGenVertexArrays(1, &m_Id));
		GL_CHECK(glBindVertexArray(m_Id));

		GL_CHECK(glGenBuffers(1, &m_VBO));
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, m_VBO));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * info.Vertices.size(), info.Vertices.data(), GL_STATIC_DRAW));

		GL_CHECK(glGenBuffers(1, &m_EBO));

		m_IndicesSize = info.Indices.size();

		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO));
		GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Index) * m_IndicesSize, info.Indices.data(), GL_STATIC_DRAW));

		GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position)));
		GL_CHECK(glEnableVertexAttribArray(0));

		if (info.HasNormals)
		{
			GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal)));
			GL_CHECK(glEnableVertexAttribArray(1));
		}
		else
		{
			GL_CHECK(glDisableVertexAttribArray(1));
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
		GL_CHECK(glDrawElements(GL_TRIANGLES, m_IndicesSize, GL_UNSIGNED_INT, 0));
	}

	void Mesh::UpdateInstanceBuffer(const std::vector<glm::mat4>& transforms)
	{
		m_InstanceSize = transforms.size();
		m_SSBO.Resize((void*) transforms.data(), transforms.size() * sizeof(glm::mat4));
	}

	void Mesh::DrawInstanced()
	{
		glBindVertexArray(m_Id);
		m_SSBO.Bind(0);
		glDrawElementsInstanced(GL_TRIANGLES, m_IndicesSize, GL_UNSIGNED_INT, 0, m_InstanceSize);
	}

	void Mesh::Unbind()
	{
		GL_CHECK(glBindVertexArray(0));
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
	}

	Mesh::~Mesh()
	{
		Unbind();

		GL_CHECK(glDeleteVertexArrays(1, &m_Id));
		GL_CHECK(glDeleteBuffers(1, &m_VBO));
		GL_CHECK(glDeleteBuffers(1, &m_EBO));
	}
}