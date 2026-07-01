#version 460 core
#extension GL_ARB_bindless_texture : require

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

struct InstanceData
{
    mat4 model;
    uint materialId;
    uint _pad0;
    uint _pad1;
    uint _pad2;
};

layout(std430, binding = 0) readonly buffer InstanceBuffer
{
    InstanceData instances[];
};

struct UnlitTextured
{
    sampler2D textureHandle;
};

layout(std430, binding = 1) readonly buffer MaterialBuffer
{
    UnlitTextured materials[];
};

uniform mat4 uViewProjection;

out vec2 vTexCoords;
flat out uint vMaterialId;

void main()
{
    uint instanceIdx = gl_BaseInstance + gl_InstanceID;
    InstanceData instance = instances[instanceIdx];
    
    vec4 worldPos = instance.model * vec4(aPosition, 1.0);
    gl_Position = uViewProjection * worldPos;
    
    vTexCoords = aTexCoords;
    vMaterialId = instance.materialId;
}