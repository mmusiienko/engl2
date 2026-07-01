#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
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
out vec3 vNormal;
out vec3 vFragPos;

void main()
{
    vec4 world = Data[gl_InstanceID].Model * vec4(aPos, 1.0);
    gl_Position = uViewProjection * world;
    vTexCoords = aTexCoords;
    vFragPos = world.xyz;
    vNormal = vec3(Data[gl_InstanceID].Normal * aNormal);
}