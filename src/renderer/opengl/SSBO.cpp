#include "../base/SSBO.h"

namespace EnGl
{
	SSBO::SSBO(const void* data, size_t size, u32 draw)
	{
		glGenBuffers(1, &m_Id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_Id);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, draw);
	}

	void SSBO::Resize(const void* data, size_t size, u32 draw) const
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_Id);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, draw);
	}

	void SSBO::Bind(u32 index) const
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, m_Id);
	}

	SSBO::~SSBO()
	{
		glDeleteBuffers(1, &m_Id);
	}
}