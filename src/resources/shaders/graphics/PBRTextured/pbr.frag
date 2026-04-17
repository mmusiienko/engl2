#version 460 core

layout(location = 0) out vec4 FragColor;

in vec3 vNormal;
in vec3 vFragPos;
in vec2 vTexCoords;
in vec4 vShadowPos;
in vec3 vTangent;

const float PI = 3.1415926;

const uint MAX_LIGHTS = 16;

struct Material
{
	sampler2D Albedo;
	sampler2D Metallic;
	sampler2D Roughness;
	sampler2D AO;
	sampler2D Normals;
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

uniform Material uMaterial;

uniform vec3 uCameraPos;

float DistributionGGX(float NDotH, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float denom = (NDotH * NDotH * (a2 - 1.0) + 1.0);
	return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float ndot, float roughness)
{
    float r = (roughness + 1.0);
    float k = r * r / 8.0;
	return ndot / (ndot * (1.0 - k) + k);
}

float GeometrySmith(float NDotV, float NDotL, float roughness)
{
	return GeometrySchlickGGX(NDotV, roughness) * GeometrySchlickGGX(NDotL, roughness);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 DirectionalLight(UniformDirectionalLight light, vec3 V, vec3 F0, vec3 albedo, float roughness, float metallic, vec3 N)
{
	vec3 L = light.Direction;
	vec3 H = normalize(V + L);
	float NDotH = max(dot(N, H), 0.0);
	float NDotV = max(dot(N, V), 0.0);
	float NDotL = max(dot(N, L), 0.0);
	float HDotV = max(dot(H, V), 0.0);

	float ndf = DistributionGGX(NDotH, roughness);
	float G = GeometrySmith(NDotV, NDotL, roughness);
	vec3 fresnel = FresnelSchlick(HDotV, F0);

	vec3 kS = fresnel;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

	vec3 num = ndf * G * fresnel;
	float denom = 4.0 * NDotV * NDotL + 0.0001;
	vec3 specular = num / denom;

	return (kD * albedo / PI + specular) * light.Color * NDotL;
}

vec3 PointLight(UniformPointLight light, vec3 V, vec3 F0, vec3 albedo, float roughness, float metallic, vec3 N)
{
	UniformDirectionalLight dirLight;
	dirLight.Direction = normalize(light.Position - vFragPos);  
	dirLight.Color = light.Color;

	float dist = length(light.Position - vFragPos);

	float attenuation = 1 / (dist * dist);    

	return DirectionalLight(dirLight, V, F0,albedo, roughness, metallic, N) * attenuation * light.Intensity;
}

const float GAUSSIAN_3X3[9] = float[9](
    1, 2, 1,
    2, 4, 2,
    1, 2, 1
);

float Shadow()
{
	vec3 proj = (vShadowPos.xyz / vShadowPos.w) * 0.5 + 0.5;

	if (proj.z > 1)
		return 0.0;

	vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
	float shadow = 0.0;
	float bias = 0.01;

	for(int x = -1; x <= 1; ++x)
	{
		for(int y = -1; y <= 1; ++y)
		{
			float depthShadowMap = texture(uShadowMap, proj.xy + vec2(x, y) * texelSize).r; 
			shadow += proj.z - bias > depthShadowMap ? GAUSSIAN_3X3[(x + 1) * 3 + (y + 1)] : 0.0;        
		}    
	}

	return shadow / 16.0;
}

const vec3 F0 = vec3(0.04); 
const vec3 AMBIENT = vec3(0.03); 

void main()
{
	vec3 L0 = vec3(0.0);

	vec3 V = normalize(uCameraPos - vFragPos);  

	vec3 albedo = texture(uMaterial.Albedo, vTexCoords).rgb;
	float roughness = texture(uMaterial.Roughness, vTexCoords).r;
	float metallic = texture(uMaterial.Metallic, vTexCoords).r;
	float ao = texture(uMaterial.AO, vTexCoords).r;

	vec3 normal = normalize(vNormal);
	vec3 tangent = normalize(vTangent);
	tangent = normalize(tangent - dot(tangent, normal) * normal);
	vec3 bitangent = normalize(cross(tangent, normal));

	vec3 normalMap = texture(uMaterial.Normals, vTexCoords).rgb;
	normalMap = normalMap * 2.0 - 1.0;
	mat3 TBN = mat3(tangent, bitangent, normal);

	vec3 N = normalize(TBN * normalMap);

    vec3 F0mix = mix(F0, albedo, metallic);

	L0 += (1.0 - Shadow()) *DirectionalLight(uDirectionalLight, V, F0mix, albedo, roughness, metallic, N);

	for (uint i = 0; i < MAX_LIGHTS; i++)
	{
		L0 += PointLight(uPointLights[i], V, F0mix, albedo, roughness, metallic, N);
	}
	
	vec3 color = AMBIENT * albedo * ao + L0;

	FragColor = vec4(color, 1.0);
}
