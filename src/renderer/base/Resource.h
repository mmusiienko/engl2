#pragma once


#include "../core/Core.h"


namespace EnGl
{
	class Resource
	{
	public:
		using ResourceId = u32;

		Resource(const Resource& other) = delete;
		Resource& operator=(const Resource& other) = delete;
		Resource(Resource&& other) noexcept : m_Id(other.m_Id)
		{
			other.m_Id = 0;
		}

		Resource& operator=(Resource&& other) noexcept
		{
			std::swap(m_Id, other.m_Id);
			return *this;
		}

		inline ResourceId Id() const 
		{
			return m_Id;
		}

		virtual ~Resource() = default;
	protected:
		Resource() = default;

		ResourceId m_Id = 0;
	};
}