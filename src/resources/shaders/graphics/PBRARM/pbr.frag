#version 460 core

layout(location = 0) out vec4 FragColor;

in vec3 vNormal;
in vec3 vFragPos;
in vec2 vTexCoords;
in vec4 vShadowPos;
in vec3 vTangent;
in vec3 vBitangent;

const float PI = 3.1415926;

const uint MAX_LIGHTS = 16;

struct Material
{
	sampler2D ARM;
	sampler2D Albedo;
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
uniform uvec2 uResolution;

float DistributionGGX(float NDotH, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float denom = (NDotH * NDotH * (a2 - 1.0) + 1.0);
	return a2 / max(PI * denom * denom, 0.001);
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

vec3 DirectionalLight(UniformDirectionalLight light, vec3 V, vec3 F0, vec3 N, float m, float r, vec3 alb)
{
	vec3 L = light.Direction;

	vec3 H = normalize(V + L);
	float NDotH = max(dot(N, H), 0.0001);
	float NDotV = max(dot(N, V), 0.0001);
	float NDotL = max(dot(N, L), 0.0001);
	float HDotV = max(dot(H, V), 0.0001);

	float ndf = DistributionGGX(NDotH, r);
	float G = GeometrySmith(NDotV, NDotL, r);
	vec3 fresnel = FresnelSchlick(HDotV, F0);

	vec3 kS = fresnel;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - m;

	vec3 num = ndf * G * fresnel;
	float denom = 4.0 * NDotV * NDotL + 0.0001;
	vec3 specular = num / denom;

	return (kD * alb / PI + specular) * light.Color * NDotL;
}

vec3 PointLight(UniformPointLight light, vec3 V, vec3 F0, vec3 N, float m, float r, vec3 alb)
{
	UniformDirectionalLight dirLight;
	dirLight.Direction = normalize(light.Position - vFragPos);  
	dirLight.Color = light.Color;

	float dist = length(light.Position - vFragPos);

	float attenuation = 1 / (dist * dist);    

	return DirectionalLight(dirLight, V, F0, N, m, r, alb) * attenuation * light.Intensity;
}


const float GAUSSIAN_3X3[9] = float[9](
    1, 2, 1,
    2, 4, 2,
    1, 2, 1
);

float Shadow(vec3 normal, UniformDirectionalLight light)
{
    vec4 s = texture(uShadowMap, gl_FragCoord.xy / vec2(uResolution));
    return s.r;
}

const vec3 F0 = vec3(0.04); 
const vec3 AMBIENT = vec3(0.03); 

void main()
{
	vec3 L0 = vec3(0.0);

	vec3 V = normalize(uCameraPos - vFragPos);  
	vec4 clr = texture(uMaterial.Albedo, vTexCoords).rgba;

	//if (clr.a < 0.5) discard;

	vec3 arm = texture(uMaterial.ARM, vTexCoords).rgb;
	vec3 albedo = clr.rgb;
	float roughness = arm.g;
	float metallic = arm.b;
	float ao = 1 - arm.r;

	vec3 normal = normalize(vNormal);
	vec3 tangent = normalize(vTangent);

	vec3 bitangent = normalize(vBitangent);
	tangent = normalize(tangent - dot(tangent, normal) * normal);
    bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	vec3 normalMap = texture(uMaterial.Normals, vTexCoords).rgb * 2.0 - 1.0;

	vec3 N = normalize(TBN * normalMap);

    vec3 F0mix = mix(F0, albedo, metallic);
	float shadow = Shadow(normal, uDirectionalLight);
	L0 += shadow * DirectionalLight(uDirectionalLight, V, F0mix, N, metallic, roughness, albedo );

	for (uint i = 0; i < MAX_LIGHTS; i++)
	{
		L0 += PointLight(uPointLights[i], V, F0mix,N, metallic, roughness, albedo);
	}
	
	vec3 color = AMBIENT * albedo * ao + L0;

	FragColor = vec4(color, 1.0);
}
