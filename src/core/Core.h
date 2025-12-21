#pragma once
#include <stdio.h>
#include <stdlib.h>

#include <memory>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stdint.h>

#include <spdlog/spdlog.h>


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