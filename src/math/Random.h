#pragma once

#include "../core/Core.h"
#include "Math.h"

namespace EnGl
{
	class Random
	{
	public:
		Random();
		Random(f32 initialState);
		f32 UniformUnit();
		f32 UniformRange(f32 min, f32 max);
		glm::vec2 UniformUnit2();
		glm::vec3 UniformUnit3();
		glm::vec2 NonUniformOnS1();
		glm::vec3 NonUniformOnS2();

		void AddToState(u32 inc) 
		{
			m_State += inc;
		}

	private:
		f32 m_State = 0;

		static u32 pcg_hash(u32 input);
	};

	class GaussianRandom : public Random
	{
	public:
		GaussianRandom();
		GaussianRandom(f32 initialState);
		glm::vec2 GaussianPair()
		{
			return GaussianPair(0.0f, 1.0f);
		}

		glm::vec2 GaussianPair(f32 mu, f32 sigma);
	};
};