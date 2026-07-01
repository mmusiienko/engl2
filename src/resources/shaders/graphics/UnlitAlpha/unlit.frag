#version 460 core

out vec4 FragColor;
in vec2 vTexCoords;

uniform vec4 uColor;
uniform sampler2D uTexture;
const float ALPHA_CUTOFF = 0.5;

void main()
{
	if (texture(uTexture, vTexCoords).a < ALPHA_CUTOFF)
	    discard;

	FragColor = uColor;
}