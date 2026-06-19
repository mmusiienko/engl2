#version 460 core

const uint MAX_LIGHTS = 16;

layout(location = 0) out vec4 FragColor;

in vec3 vNormal;
in vec3 vFragPos;
in vec2 vTexCoords;
in vec3 vTangent;
in vec3 vBitangent;

struct Material
{
	sampler2D Diffuse;
	sampler2D Specular;
	sampler2D Normals;
	float Shininess;
};

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

uniform UniformPointLight uLights[MAX_LIGHTS];
uniform UniformDirectionalLight uDirectionalLight;

uniform sampler2D uShadowMap;
uniform uvec2 uResolution;

uniform Material uMaterial;

uniform vec3 uCameraPos;

vec3 DirectionalLight(UniformDirectionalLight light, vec3 objDiff, vec3 objSpecular, vec3 viewDir, vec3 norm)
{
	float diff = max(dot(norm, normalize(light.Direction)), 0.0);
	vec3 diffuse = light.Color * (diff * objDiff);

	vec3 reflectVec = reflect(-light.Direction, norm);
	float spec = pow(max(dot(reflectVec, viewDir), 0.0), uMaterial.Shininess);
	vec3 specular = spec * objSpecular;

	return diffuse + specular;
}

vec3 PointLight(UniformPointLight light, vec3 objDiff, vec3 objSpecular, vec3 viewDir, vec3 normal)
{
	UniformDirectionalLight dirLight;
	dirLight.Direction = normalize(light.Position - vFragPos);  
	dirLight.Color = light.Color;

	float dist = length(light.Position - vFragPos);

	float attenuation = pow(light.Intensity / max(dist, 0.01), 2);    

	return DirectionalLight(dirLight, objDiff, objSpecular, viewDir, normal) * attenuation;
}

float Shadow(vec3 normal, UniformDirectionalLight light)
{
    vec4 s = texture(uShadowMap, gl_FragCoord.xy / vec2(uResolution));
    return s.r;
}

void main()
{
	vec3 outputColor = vec3(0.0);

	vec4 diffuseT = texture(uMaterial.Diffuse, vTexCoords);
	
	vec3 diffuse = diffuseT.xyz;

	vec3 specular = texture(uMaterial.Specular, vTexCoords).xyz;

	vec3 normal = normalize(vNormal);
	vec3 tangent = normalize(vTangent);
	vec3 bitangent = normalize(vBitangent);
	tangent = normalize(tangent - dot(tangent, normal) * normal);
    bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	vec3 normalMap = texture(uMaterial.Normals, vTexCoords).rgb * 2.0 - 1.0;

	vec3 N = normalize(TBN * normalMap);
	float shadow = Shadow(normal, uDirectionalLight);

	vec3 viewDir = normalize(uCameraPos - vFragPos);  

	outputColor += shadow * DirectionalLight(uDirectionalLight, diffuse, specular, viewDir, N);

	for (uint i = 0; i < MAX_LIGHTS; i++)
	{
		outputColor += PointLight(uLights[i], diffuse, specular, viewDir, N);
	}
	outputColor += diffuse * 0.3;

	FragColor = vec4(outputColor, diffuseT.a);
}
