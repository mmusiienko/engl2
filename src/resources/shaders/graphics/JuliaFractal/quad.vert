#version 460

layout(location = 0) in vec3 aPos;
layout(location = 2) in vec2 aTexCoords;

out vec2 vTexCoords;

uniform mat4 uModel;

void main()
{
	gl_Position = uModel * vec4(aPos, 1.0);
	vTexCoords = aTexCoords;
}
