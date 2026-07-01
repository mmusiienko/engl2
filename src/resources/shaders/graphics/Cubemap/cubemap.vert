#version 460 core

layout(location = 0) in vec3 aPos;

uniform mat4 uViewProjection;

out vec3 vTexCoords;

void main()
{
    vTexCoords = aPos;

    vec4 pos = uViewProjection * vec4(aPos, 1.0);
    gl_Position = vec4(pos.xy, 0.0, pos.w);
}  