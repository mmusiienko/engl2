#version 460 core

out vec4 FragColor;
in vec2 vTexCoords;
in vec3 vFragPos;
in vec3 vNormal;

uniform vec4 uColor;

struct Color
{
	vec3 Ambient;
	vec3 Diffuse;
	vec3 Specular;
};

struct Attenuation
{
	float Intensity;
};

struct UniformPointLight
{
	vec3 Position;
	Color Color;
	Attenuation Attenuation;
};

struct UniformDirectionalLight
{
	vec3 Direction;
	Color Color;
};

//uniform UniformPointLight uLights[MAX_LIGHTS];
uniform UniformDirectionalLight uDirectionalLight = UniformDirectionalLight(normalize(vec3(1.0, -1.0, 0)), Color(vec3(0.3), vec3(0.8), vec3(1.0,1.0,1.0)));
	
uniform vec3 uCameraPos;

vec3 DirectionalLight(UniformDirectionalLight light, vec3 objColor, vec3 viewDir)
{
	vec3 lightSpecular = light.Color.Specular;

	vec3 ambient = light.Color.Ambient * objColor;

	vec3 norm = normalize(vNormal);
	float diff = max(dot(norm, normalize(light.Direction)), 0.0);
	vec3 diffuse = light.Color.Diffuse * (diff * objColor);

	vec3 reflectVec = reflect(-light.Direction, norm);
	float spec = pow(max(dot(reflectVec, viewDir), 0.0), 32.0);
	vec3 specular = lightSpecular * (spec * objColor);

	return diffuse + ambient;
}

void main()
{
	vec3 color = uColor.rgb;
	vec3 viewDir = normalize(uCameraPos - vFragPos);
	color = DirectionalLight(uDirectionalLight, color, viewDir);
	FragColor = vec4(color, 1.0);
}