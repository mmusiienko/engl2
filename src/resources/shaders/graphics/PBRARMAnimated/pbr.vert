#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in ivec4 aBoneIds;
layout (location = 6) in vec4 aWeights;

layout(std430, binding = 0) buffer BoneMatrices {
    mat4 Transform[];
};

out vec3 vFragPos;
out vec2 vTexCoords;

out vec3 vNormal;
out vec3 vTangent;
out vec3 vBitangent;

uniform mat4 uModel;
uniform mat3x4 uNormal;
uniform mat4 uViewProjection;

const uint MAX_BONE_INFLUENCE = 4;

void main()
{
    vec4 pos = vec4(0.0);
    vec3 normal = vec3(0.0);
    vec3 tangent = vec3(0.0);
    vec3 bitangent = vec3(0.0);

    bool hasBones = false;

    for (uint i = 0; i < MAX_BONE_INFLUENCE; i++)
    {
        if (aBoneIds[i] == -1) break;

        pos += aWeights[i] * Transform[aBoneIds[i]] * vec4(aPos, 1.0);

        mat3 boneRot = mat3(Transform[aBoneIds[i]]);
        normal += aWeights[i] * boneRot * aNormal;
        tangent += aWeights[i] * boneRot * aTangent;
        bitangent += aWeights[i] * boneRot * aBitangent;
        hasBones = true;
    }

    if (!hasBones)
    {
        pos = vec4(aPos, 1.0);
        normal = aNormal;
        tangent = aTangent;
        bitangent = aBitangent;
    }

    vec4 worldPos = uModel * pos;

    gl_Position = uViewProjection * worldPos;

    vNormal = normalize(vec3(uNormal * normal));
    vTangent = normalize(vec3(uNormal * tangent));
    vBitangent = normalize(vec3(uNormal * bitangent));

    vFragPos = worldPos.xyz;

    vTexCoords = aTexCoords;
}