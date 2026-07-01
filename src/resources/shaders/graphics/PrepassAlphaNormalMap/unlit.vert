#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

uniform mat4 uModel;
uniform mat3x4 uNormal;
uniform mat4 uViewProjection;

out vec3 vNormal;
out vec3 vTangent;
out vec2 vTexCoords;

void main()
{
    gl_Position = uViewProjection * uModel * vec4(aPos, 1.0);
    vNormal = normalize(vec3(uNormal * aNormal));
    vTangent = normalize(vec3(uNormal * aTangent));
    vTexCoords = aTexCoords;
}