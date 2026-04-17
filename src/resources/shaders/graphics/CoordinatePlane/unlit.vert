#version 460 core
layout (location = 0) in vec3 aPos;

uniform mat4 uModel;
uniform mat4 uViewProjection;
uniform vec3 uCameraPos;

out vec3 vWorldPos;

void main()
{
    vec4 worldPos = uModel * vec4(aPos, 1.0);
    worldPos.y = 1;
    worldPos.xz += uCameraPos.xz;
    gl_Position = uViewProjection * worldPos;
    vWorldPos = vec3(worldPos);
}