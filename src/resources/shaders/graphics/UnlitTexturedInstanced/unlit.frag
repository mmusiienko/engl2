#version 460 core

out vec4 FragColor;
in vec2 vTexCoords;

uniform sampler2D uTexture;

void main()
{
	FragColor = texture(uTexture, vTexCoords);
}