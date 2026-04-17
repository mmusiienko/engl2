#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
out vec3 vNormal;
out vec3 vFragPos;
out vec2 vTexCoords;

uniform mat4 uModel;
uniform mat3x4 uNormal;
uniform mat4 uViewProjection;

void main()
{
    vec4 pos = uModel * vec4(aPos, 1.0);

    gl_Position = uViewProjection * pos;
    vFragPos = pos.xyz;
    vNormal = vec3(uNormal * aNormal);
    vTexCoords = aTexCoords;
}