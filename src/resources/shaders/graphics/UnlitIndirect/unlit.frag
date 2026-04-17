#version 460 core

struct Unlit
{
    vec4 Color;
};

layout(std430, binding = 1) readonly buffer MaterialBuffer
{
    Unlit materials[];
};

in vec2 vTexCoords;
flat in uint vMaterialId;

out vec4 FragColor;

void main()
{
    Unlit material = materials[vMaterialId];
    
    FragColor = material.Color;
}