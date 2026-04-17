#version 460 core
#extension GL_ARB_bindless_texture : require

struct UnlitTextured
{
    sampler2D textureHandle;
};

layout(std430, binding = 1) readonly buffer MaterialBuffer
{
    UnlitTextured materials[];
};

in vec2 vTexCoords;
flat in uint vMaterialId;

out vec4 FragColor;

void main()
{
    UnlitTextured material = materials[vMaterialId];
    
    vec4 texColor = texture(material.textureHandle, vTexCoords);
    
    FragColor = texColor;
}