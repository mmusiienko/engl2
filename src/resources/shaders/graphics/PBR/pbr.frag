#version 460 core

layout(location = 0) out vec4 FragColor;

in vec3 vNormal;
in vec3 vFragPos;
in vec2 vTexCoords;
in vec4 vShadowPos;

const float PI = 3.1415926;

const uint MAX_LIGHTS = 16;

struct Material
{
	vec3 Albedo;
	float Metallic;
	float Roughness;
	float AO;
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

uniform UniformPointLight uPointLights[MAX_LIGHTS];
uniform UniformDirectionalLight uDirectionalLight;
uniform sampler2D uShadowMap;
uniform uvec2 uResolution;

uniform Material uMaterial;

uniform vec3 uCameraPos;

float DistributionGGX(float NDotH)
{
	float a = uMaterial.Roughness * uMaterial.Roughness;
	float a2 = a * a;
	float denom = (NDotH * NDotH * (a2 - 1.0) + 1.0);
	return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float ndot)
{
    float r = (uMaterial.Roughness + 1.0);
    float k = r * r / 8.0;
	return ndot / (ndot * (1.0 - k) + k);
}

float GeometrySmith(float NDotV, float NDotL)
{
	return GeometrySchlickGGX(NDotV) * GeometrySchlickGGX(NDotL);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 DirectionalLight(UniformDirectionalLight light, vec3 V, vec3 F0)
{
	vec3 L = light.Direction;

	vec3 H = normalize(V + L);
	vec3 N = normalize(vNormal);
	float NDotH = max(dot(N, H), 0.0001);
	float NDotV = max(dot(N, V), 0.0001);
	float NDotL = max(dot(N, L), 0.0001);
	float HDotV = max(dot(H, V), 0.0001);

	float ndf = DistributionGGX(NDotH);
	float G = GeometrySmith(NDotV, NDotL);
	vec3 fresnel = FresnelSchlick(HDotV, F0);

	vec3 kS = fresnel;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - uMaterial.Metallic;

	vec3 num = ndf * G * fresnel;
	float denom = 4.0 * NDotV * NDotL + 0.0001;
	vec3 specular = num / denom;

	return (kD * uMaterial.Albedo / PI + specular) * light.Color * NDotL;
}

vec3 PointLight(UniformPointLight light, vec3 V, vec3 F0)
{
	UniformDirectionalLight dirLight;
	dirLight.Direction = normalize(light.Position - vFragPos);  
	dirLight.Color = light.Color;

	float dist = length(light.Position - vFragPos);

	float attenuation = 1 / (dist * dist);    

	return DirectionalLight(dirLight, V, F0) * attenuation * light.Intensity;
}

const float GAUSSIAN_3X3[9] = float[9](
    1, 2, 1,
    2, 4, 2,
    1, 2, 1
);

float Shadow(vec3 normal, UniformDirectionalLight light, vec2 screenUv)
{
    vec4 s = texture(uShadowMap, screenUv);
    return s.r;
}

float SSAO(vec2 screenUv)
{
    vec4 s = texture(uSSAO, screenUv);
    return s.r;
}

const vec3 F0 = vec3(0.04); 
const vec3 AMBIENT = vec3(0.03); 

void main()
{
	vec2 screenUv = gl_FragCoord.xy / vec2(uResolution);
	vec3 L0 = vec3(0.0);

	vec3 V = normalize(uCameraPos - vFragPos);  

    vec3 F0mix = mix(F0, uMaterial.Albedo, uMaterial.Metallic);
	float shadow = Shadow(normal, uDirectionalLight, screenUv);
	L0 += shadow * DirectionalLight(uDirectionalLight, V, F0mix);

	for (uint i = 0; i < MAX_LIGHTS; i++)
	{
		L0 += PointLight(uPointLights[i], V, F0mix);
	}
	
	vec3 color = AMBIENT * uMaterial.Albedo * uMaterial.AO + L0;
	FragColor = vec4(color, 1.0);
}
