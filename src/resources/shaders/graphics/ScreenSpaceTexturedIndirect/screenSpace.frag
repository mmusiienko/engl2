#version 460

in vec2 vTexCoords;
out vec4 FragColor;

uniform sampler2D uTexture;

void main()
{
	FragColor = texture(uTexture, vTexCoords);
}
