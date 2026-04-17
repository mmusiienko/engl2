#version 460 core

const uint MAX_LIGHTS = 16;

layout(location = 0) out vec4 FragColor;

in vec3 vNormal;
in vec3 vFragPos;
in vec2 vTexCoords;

struct Material
{
	sampler2D Diffuse;
	sampler2D Specular;

	float Shininess;
};

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

uniform UniformPointLight uLights[MAX_LIGHTS];
uniform UniformDirectionalLight uDirectionalLight = UniformDirectionalLight(normalize(vec3(1.0, -1.0, 0)), Color(vec3(0.3), vec3(0.8), vec3(1.0,1.0,1.0)));
	
uniform Material uMaterial;

uniform vec3 uCameraPos;

vec3 DirectionalLight(UniformDirectionalLight light, Color objColor, vec3 viewDir)
{
	vec3 lightSpecular = light.Color.Specular;

	vec3 ambient = light.Color.Ambient * objColor.Ambient;

	vec3 norm = normalize(vNormal);
	float diff = max(dot(norm, normalize(light.Direction)), 0.0);
	vec3 diffuse = light.Color.Diffuse * (diff * objColor.Diffuse);

	vec3 reflectVec = reflect(-light.Direction, norm);
	float spec = pow(max(dot(reflectVec, viewDir), 0.0), uMaterial.Shininess);
	vec3 specular = lightSpecular * (spec * objColor.Specular);

	return ambient + diffuse + specular;
}

vec3 PointLight(UniformPointLight light, Color objColor, vec3 viewDir)
{
	UniformDirectionalLight dirLight;
	dirLight.Direction = normalize(light.Position - vFragPos);  
	dirLight.Color = light.Color;

	float dist = length(light.Position - vFragPos);

	float attenuation = pow(light.Attenuation.Intensity / max(dist, 0.01), 2);    

	return DirectionalLight(dirLight, objColor, viewDir) * attenuation;
}

void main()
{
	vec3 outputColor = vec3(0.0);

	vec4 diffuseT = texture(uMaterial.Diffuse, vTexCoords);
	
	vec3 diffuse = diffuseT.xyz;

	vec3 specular = texture(uMaterial.Specular, vTexCoords).xyz;

	Color color;
	color.Ambient = diffuse;
	color.Diffuse = diffuse;
	color.Specular = specular;

	vec3 viewDir = normalize(uCameraPos - vFragPos);  

	outputColor += DirectionalLight(uDirectionalLight, color, viewDir);

	for (uint i = 0; i < MAX_LIGHTS; i++)
	{
		outputColor += PointLight(uLights[i], color, viewDir);
	}
	
	FragColor = vec4(outputColor, diffuseT.a);
}
