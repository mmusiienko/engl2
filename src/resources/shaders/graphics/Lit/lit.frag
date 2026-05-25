#version 460 core

out vec4 FragColor;
in vec2 vTexCoords;
in vec3 vFragPos;
in vec3 vNormal;

uniform vec4 uColor;

struct UniformPointLight
{
	vec3 Position;
	vec3 Color;
	float Intensity;
};

struct UniformDirectionalLight
{
	vec3 Direction;
	vec3 Color;
};

//uniform UniformPointLight uLights[MAX_LIGHTS];
uniform UniformDirectionalLight uDirectionalLight;
	
uniform vec3 uCameraPos;

vec3 DirectionalLight(UniformDirectionalLight light, vec3 objColor, vec3 viewDir)
{
	vec3 ambient = 0.04 * objColor;

	vec3 norm = normalize(vNormal);
	float diff = max(dot(norm, normalize(light.Direction)), 0.0);
	vec3 diffuse = light.Color * (diff * objColor);

	vec3 reflectVec = reflect(-light.Direction, norm);
	float spec = pow(max(dot(reflectVec, viewDir), 0.0), 32.0);
	vec3 specular = light.Color * (spec * objColor);

	return diffuse + ambient;
}

void main()
{
	vec3 color = uColor.rgb;

	vec3 viewDir = normalize(uCameraPos - vFragPos);
	color = DirectionalLight(uDirectionalLight, color, viewDir);
	FragColor = vec4(color, 1.0);
}