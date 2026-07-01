#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 5) in ivec4 aBoneIds;
layout (location = 6) in vec4 aWeights;

layout(std430, binding = 0) readonly buffer BoneMatrices {
    mat4 Transform[];
};

uniform mat4 uModel;
uniform mat4 uViewProjection;

const uint MAX_BONE_INFLUENCE = 4;

void main()
{
    vec4 pos = vec4(0.0);
    bool hasBones = false;

    for (uint i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if (aBoneIds[i] == -1) break;

        pos += aWeights[i] * Transform[aBoneIds[i]] * vec4(aPos, 1.0);
        hasBones = true;
    }

    if (!hasBones)
        pos = vec4(aPos, 1.0);

    gl_Position = uViewProjection * uModel * pos;
}