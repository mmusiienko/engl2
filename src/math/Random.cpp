#include "math/Random.h"

#include <limits>


namespace EnGl
{
    Random::Random() : Random(0) { }
    Random::Random(u32 initialState) : m_State(initialState) {  }
    GaussianRandom::GaussianRandom() : Random() {}
    GaussianRandom::GaussianRandom(u32 initialState) : Random(initialState) {}

    u32 Random::pcg_hash(u32 input)
    {
        u32 state = input * 747796405u + 2891336453u;
        u32 word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
        return (word >> 22u) ^ word;
    }

    f32 Random::UniformUnit()
    {
        static f32 invMaxFloat = 1.0f / std::numeric_limits<u32>::max();

        m_State = pcg_hash(m_State);

        return static_cast<f32>(m_State) * invMaxFloat;
    }

    f32 Random::UniformRange(f32 min, f32 max)
    {
        return UniformUnit() * (max - min) + min;
    }

    glm::vec2 Random::UniformUnit2()
    {
        return { UniformUnit(), UniformUnit() };
    }

    glm::vec3 Random::UniformUnit3()
    {
        return { UniformUnit(), UniformUnit(), UniformUnit() };
    }

    glm::vec2 Random::NonUniformOnS1()
    {
        return { UniformRange(-1, 1), UniformRange(-1, 1) };
    }

    glm::vec3 Random::NonUniformOnS2()
    {
        return { UniformRange(-1, 1), UniformRange(-1, 1), UniformRange(-1, 1) };
    }

    glm::vec2 GaussianRandom::GaussianPair(f32 mu, f32 sigma)
    {
        auto uniformFloat2 = UniformUnit2();

        f32 a = sigma * static_cast<f32>(glm::sqrt(-2.0 * glm::log(1.0 - uniformFloat2.x)));
        f32 b = Math::TWO_PI * uniformFloat2.y;

        return glm::vec2(glm::cos(b), glm::sin(b)) * a + mu;
    }
}