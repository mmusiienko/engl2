#pragma once
#include "../../math/Math.h"
#include <vector>


namespace EnGl
{
	class DebugMesh
	{
	public:
		static constexpr inline glm::vec4 DEFAULT_COLOR{ 1.0f, 0.0f, 0.0f, 1.0f };
		static constexpr inline f32 DEFAULT_WIDTH = 0.1f;

		DebugMesh();
		~DebugMesh();
		void Line(const glm::vec3& p, const glm::vec3& q, const glm::vec4& color = DEFAULT_COLOR, f32 width = DEFAULT_WIDTH);
		void Quad(const glm::vec3& p, const glm::vec3& q, const glm::vec3& a, const glm::vec3& b, const glm::vec4& color = DEFAULT_COLOR);
		void FrameQuad(const glm::vec3& p, const glm::vec3& q, const glm::vec3& a, const glm::vec3& b, const glm::vec4& color = DEFAULT_COLOR, f32 width = DEFAULT_WIDTH);
		void Cube(const glm::vec3& p, const glm::vec3& q, const glm::vec4& color = DEFAULT_COLOR);
		void FrameCube(const glm::vec3& p, const glm::vec3& q, const glm::vec4& color = DEFAULT_COLOR, f32 width = DEFAULT_WIDTH);

		void Tick();
		
		void Draw();
	private:
		struct DebugVertex
		{
			glm::vec3 Position;
			glm::vec4 Color;
		};

		u32 m_VAO = 0;
		u32 m_VBO = 0;

		std::vector<DebugVertex> m_Vertices;
	};
}

