#pragma once
#include "Resource.h"


namespace EnGl
{
	class SSBO : public Resource
	{
	public:
		SSBO(const void* data, size_t size, u32 draw = GL_DYNAMIC_DRAW);
		SSBO(SSBO&& other) = default;
		SSBO& operator=(SSBO&& other) = default;

		void Bind(u32 index) const;
		void Resize(const void* data, size_t size, u32 draw = GL_DYNAMIC_DRAW) const;
		virtual ~SSBO();
	};
}