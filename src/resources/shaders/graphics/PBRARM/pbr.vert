#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out vec3 vFragPos;
out vec2 vTexCoords;
out vec4 vShadowPos;

out vec3 vTCamPos;

out vec3 vNormal;
out vec3 vTangent;

uniform mat4 uModel;
uniform mat3x4 uNormal;
uniform mat4 uViewProjection;
uniform mat4 uShadowMapViewProjection;

void main()
{
    vec4 pos = uModel * vec4(aPos, 1.0);

    vShadowPos = uShadowMapViewProjection * pos;
    gl_Position = uViewProjection * pos;
    vNormal = normalize(vec3(uNormal * aNormal));
    vTangent = normalize(vec3(uModel * vec4(aTangent, 1.0)));

    vFragPos = pos.xyz;

    vTexCoords = aTexCoords;
}