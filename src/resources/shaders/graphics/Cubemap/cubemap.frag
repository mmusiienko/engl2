#version 460 core

out vec4 FragColor;

in vec3 vTexCoords;
uniform samplerCube uCubemap;

void main()
{
	FragColor = texture(uCubemap, vTexCoords);
}
