#include "DebugMesh.h"
#include "../../Printer.h"


namespace EnGl
{
	DebugMesh::DebugMesh()
	{
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);

		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)0);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, Color));
	}

	DebugMesh::~DebugMesh()
	{
		glDeleteVertexArrays(1, &m_VAO);
		glDeleteBuffers(1, &m_VBO);
	}

	static std::pair<glm::vec3, glm::vec3> BuildOrthonormalBasis(const glm::vec3& unit)
	{
		if (unit.z < -0.999805696)
		{
			return { {0, -1, 0}, {-1, 0, 0} };
		}

		f32 a = 1.0f / (1.0f + unit.z);
		f32 ya = unit.y * a;
		f32 d = -unit.x * ya;


		return { 
			{1.0f - unit.x * unit.x * a, d, -unit.x},
			{d, 1.0f - unit.y * ya, -unit.y}
		};
	}

	void DebugMesh::Line(const glm::vec3& p, const glm::vec3& q, const glm::vec4& color, f32 width)
	{
		glm::vec3 unit = glm::normalize(p - q);
		auto [a, b] = BuildOrthonormalBasis(unit);

		glm::vec3 halfA = (width * 0.5f) * a;
		glm::vec3 halfB = (width * 0.5f) * b;

		glm::vec3 v0 = p + halfA + halfB;
		glm::vec3 v1 = p - halfA + halfB;
		glm::vec3 v2 = p - halfA - halfB;
		glm::vec3 v3 = p + halfA - halfB;

		glm::vec3 v4 = q + halfA + halfB;
		glm::vec3 v5 = q - halfA + halfB;
		glm::vec3 v6 = q - halfA - halfB;
		glm::vec3 v7 = q + halfA - halfB;

		Quad(v2, v1, v0, v3, color);
		Quad(v6, v5, v4, v7, color);

		Quad(v0, v1, v5, v4, color);
		Quad(v2, v3, v7, v6, color);

		Quad(v3, v0, v4, v7, color);
		Quad(v6, v5, v1, v2, color);
	}

	void DebugMesh::Quad(const glm::vec3& p, const glm::vec3& q, const glm::vec3& a, const glm::vec3& b, const glm::vec4& color)
	{
		m_Vertices.push_back({ p, color });
		m_Vertices.push_back({ q, color });
		m_Vertices.push_back({ a, color });
		m_Vertices.push_back({ a, color });
		m_Vertices.push_back({ b, color });
		m_Vertices.push_back({ p, color });
	}

	void DebugMesh::Cube(const glm::vec3& p, const glm::vec3& q, const glm::vec4& color)
	{
		
	}

	void DebugMesh::FrameQuad(const glm::vec3& p, const glm::vec3& q, const glm::vec3& a, const glm::vec3& b, const glm::vec4& color, f32 width)
	{
		Line(p, q, color, width);
		Line(q, a, color, width);
		Line(a, b, color, width);
		Line(b, p, color, width);
	}

	void DebugMesh::FrameCube(const glm::vec3& p, const glm::vec3& q, const glm::vec4& color, f32 width)
	{
		glm::vec3 w = q - p;

		glm::vec3 p1 = { p.x + w.x, p.y, p.z };
		glm::vec3 p2 = { p.x, p.y, p.z + w.z };
		glm::vec3 p3 = { p.x + w.x, p.y, p.z + w.z };

		glm::vec3 q0 = { p.x, p.y + w.y, p.z };
		glm::vec3 q1 = { p1.x, p.y + w.y, p1.z };
		glm::vec3 q2 = { p2.x, p.y + w.y, p2.z };

		FrameQuad(p, p1, p3, p2, color, width);
		FrameQuad(q0, q1, q, q2, color, width);

		Line(p, q0, color, width);
		Line(p1, q1, color, width);
		Line(p2, q2, color, width);
		Line(p3, q, color, width);
	}

	void DebugMesh::Tick()
	{
		m_Vertices.clear();
	}

	void DebugMesh::Draw()
	{
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glBufferData(GL_ARRAY_BUFFER, m_Vertices.size() * sizeof(DebugVertex), (void*) m_Vertices.data(), GL_DYNAMIC_DRAW);

		glDrawArrays(GL_TRIANGLES, 0, m_Vertices.size());
	}
}

