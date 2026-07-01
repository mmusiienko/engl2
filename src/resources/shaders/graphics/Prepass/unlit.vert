#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 uModel;
uniform mat3x4 uNormal;
uniform mat4 uViewProjection;

out vec3 vNormal;

void main()
{
    gl_Position = uViewProjection * uModel * vec4(aPos, 1.0);
    vNormal = normalize(vec3(uNormal * aNormal));
}