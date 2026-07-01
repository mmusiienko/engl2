#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoords;

struct InstanceData
{
    mat4 Model;
    mat3x4 Normal;
};

layout(std430, binding = 0) readonly buffer Buff
{
    InstanceData Data[];
};

uniform mat4 uViewProjection;

out vec2 vTexCoords;

void main()
{
    vec4 world = Data[gl_InstanceID].Model * vec4(aPos, 1.0);
    gl_Position = uViewProjection * world;
    vTexCoords = aTexCoords;
}