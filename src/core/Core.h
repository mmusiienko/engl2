#pragma once

#include <stdio.h>
#include <stdlib.h>

#include <memory>

#include <stdint.h>
#include <spdlog/spdlog.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

template<class T>
using ref = std::shared_ptr<T>;

template<class T, class... Args>
ref<T> make_ref(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template<class T>
using scope = std::unique_ptr<T>;

template<class T, class... Args>
scope<T> make_scope(Args&&... args)
{
    return std::make_unique<T>(std::forward<Args>(args)...);
}

inline void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        printf("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
        abort();
    }
}

#ifdef _DEBUG
#define GL_CHECK(stmt) do { \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK(stmt) stmt
#endif

template <>
struct fmt::formatter<glm::vec3> {
    constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }
    auto format(const glm::vec3& v, fmt::format_context& ctx) const {
        return fmt::format_to(ctx.out(), "vec3(x:{}, y:{}, z:{})", v.x, v.y, v.z);
    }
};

template <>
struct fmt::formatter<glm::vec4> {
    constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }
    auto format(const glm::vec4& v, fmt::format_context& ctx) const {
        return fmt::format_to(ctx.out(), "vec4(x:{}, y:{}, z:{}, w:{})", v.x, v.y, v.z, v.w);
    }
};

template <>
struct fmt::formatter<glm::quat> {
    constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }
    auto format(const glm::quat& v, fmt::format_context& ctx) const {
        return fmt::format_to(ctx.out(), "quat(x:{}, y:{}, z:{}, w:{})", v.x, v.y, v.z, v.w);
    }
};

template <>
struct fmt::formatter<glm::mat3> {
    constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }
    auto format(const glm::mat3& m, fmt::format_context& ctx) const {
        return fmt::format_to(ctx.out(),
            "mat3(\n  col0:{}\n  col1:{}\n  col2:{})",
            m[0], m[1], m[2]
        );
    }
};

template <>
struct fmt::formatter<glm::mat4> {
    constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }
    auto format(const glm::mat4& m, fmt::format_context& ctx) const {
        return fmt::format_to(ctx.out(),
            "mat4(\n  col0:{}\n  col1:{}\n  col2:{}\n  col3:{})",
            m[0], m[1], m[2], m[3]
        );
    }
};

namespace EnGl
{
    typedef uint32_t u32;
    typedef uint64_t u64;
    typedef int32_t i32;
    typedef int64_t i64;
    typedef float f32;
    typedef double f64;

    template <class T>
    inline void hash_combine(std::size_t& s, const T& v)
    {
        s ^= std::hash<T>{}(v)+0x9e3779b9 + (s << 6) + (s >> 2);
    }
}