#version 460 core
out vec4 FragColor;

in vec3 vFragPos;
in vec2 vPos;
in flat int vLod;
in vec4 vShadowPos;

uniform vec3 uCameraPos;
uniform float uNear;
uniform float uFar;
uniform float uTime;

const float PI = 3.1415926;

const uint MAX_LIGHTS = 16;

struct Material
{
    vec3  Albedo;
    float Metallic;
    float Roughness;
    float AO;
};

struct UniformPointLight
{
    vec3  Position;
    vec3  Color;
    float Intensity;
};

struct UniformDirectionalLight
{
    vec3 Direction;
    vec3 Color;
};

uniform UniformPointLight       uPointLights[MAX_LIGHTS];
uniform UniformDirectionalLight uDirectionalLight;
uniform sampler2D               uShadowMap;
uniform uvec2 uResolution;

uniform Material material = Material(vec3(0.1, 0.8, 0.1), 0.0, 0.6, 1.0);

float DistributionGGX(float NDotH, float roughness)
{
    float a  = roughness * roughness;
    float a2 = a * a;
    float d  = NDotH * NDotH * (a2 - 1.0) + 1.0;
    return a2 / (PI * d * d);
}

float GeometrySchlickGGX(float ndot, float roughness)
{
    float r = roughness + 1.0;
    float k = r * r / 8.0;
    return ndot / (ndot * (1.0 - k) + k);
}

float GeometrySmith(float NDotV, float NDotL, float roughness)
{
    return GeometrySchlickGGX(NDotV, roughness) * GeometrySchlickGGX(NDotL, roughness);
}

vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 DirectionalLight(UniformDirectionalLight light, vec3 V, vec3 F0mat,
                           vec3 N, vec3 albedo)
{
    vec3  L     = light.Direction;
    vec3  H     = normalize(V + L);
    float NDotH = max(dot(N, H), 0.0);
    float NDotV = max(dot(N, V), 0.0);
    float NDotL = max(dot(N, L), 0.0);
    float HDotV = max(dot(H, V), 0.0);

    float ndf     = DistributionGGX(NDotH, material.Roughness);
    float G       = GeometrySmith(NDotV, NDotL, material.Roughness);
    vec3  fresnel = FresnelSchlick(HDotV, F0mat);

    vec3 kD      = (vec3(1.0) - fresnel) * (1.0 - material.Metallic);
    vec3 specular = (ndf * G * fresnel) / (4.0 * NDotV * NDotL + 0.0001);

    return 10 * (kD * albedo / PI + specular) * light.Color * NDotL;
}  

vec3 PointLight(UniformPointLight light, vec3 V, vec3 F0mat, vec3 N, vec3 albedo)
{
    UniformDirectionalLight dirLight;
    dirLight.Direction = normalize(light.Position - vFragPos);
    dirLight.Color     = light.Color;

    float dist        = length(light.Position - vFragPos);
    float attenuation = 1.0 / (dist * dist);

    return DirectionalLight(dirLight, V, F0mat, N, albedo) * attenuation * light.Intensity;
}

const float GAUSSIAN_3X3[9] = float[9](
    1.0, 2.0, 1.0,
    2.0, 4.0, 2.0,
    1.0, 2.0, 1.0
);

float Shadow(vec3 normal, UniformDirectionalLight light)
{
    vec4 s = texture(uShadowMap, gl_FragCoord.xy / vec2(uResolution));
    return s.r;
}

const vec3 F0 = vec3(0.04);
const vec3 AMBIENT  = vec3(0.7);

uniform sampler2D uTerrainInfo;
uniform float uTerrainResolution = 1.0;
uniform vec4 uHeightWeights = vec4(100.0, 1.0, 0.1, 0.001);
uniform vec4 uScale = vec4(0.001);
const float baseScale = 0.001;

const vec3 GRASS = vec3(0.1, 0.8, 0.1) * 0.2;
const vec3 SNOW = vec3(1.0) * 0.2;
const vec3 SAND = vec3(224.0 / 255.0, 198.0 / 255.0, 49.0 / 255.0);
const vec3 ROCK = vec3(0.03) ;
const vec3 ROCK2 = vec3(0.2, 0.05, 0.07);

float remap(float val, float mn1, float mx1, float mn2, float mx2)
{
	return (val - mn1) / (mx1 - mn1) * (mx2 - mn2) + mn2;
}

float saturate(float val)
{
	return clamp(val, 0, 1);
}

float grad(float val, float mn1, float mx1, float mn2, float mx2)
{
	return saturate(remap(val, mn1, mx1, mn2, mx2));
}

vec3 getColor(float height)
{
    vec4 s = texture(uTerrainInfo, vPos * baseScale * uScale.a * 0.5);

    float h = height + s.a * 100;
    //float sandToGrass = grad(h, 0.0, 50.0, 0.0, 1.0);
    //float grassToRock = grad(h,   30.0, 100.0, 0.0, 1.0);
    //float rockToSnow  = grad(h,  200.0, 400.0, 0.0, 1.0);

    float sandToRock = grad(h, -600.0, -300.0, 0.0, 1.0);
    float rockToGrass = grad(h,   -300.0, 000.0, 0.0, 1.0);

    vec3 col = SAND;
    col = mix(col, ROCK, sandToRock);
    col = mix(col, ROCK2,  rockToGrass);
    return col + s.rgb * vec3(1.0, 0.8, 0.7) * 0.01;
}

vec3 getColorAngle(float angle)
{
    float h = angle;
    float sandToGrass = grad(h, 0.0, 0.1, 0.0, 1.0);
    float grassToRock = grad(h,   0.05, 0.3, 0.0, 1.0);
    float rockToSnow  = grad(h,  0.8, 1.0, 0.0, 1.0);

    vec3 col = SAND;
    col = mix(col, GRASS, sandToGrass);
    col = mix(col, ROCK,  grassToRock);
    col = mix(col, SNOW,  rockToSnow);
    return col;
}

in float vHeight;
in vec3 vNormal;

void main()
{
    vec3 normal = normalize(vNormal);

    vec3  V     = normalize(uCameraPos - vFragPos);

    vec3 albedo = getColor(vHeight);

    vec3 F0mix = mix(F0, albedo, material.Metallic);

    vec3 L0 = vec3(0.0);
    float shadow = Shadow(normal, uDirectionalLight);
    L0 += shadow * DirectionalLight(uDirectionalLight, V, F0mix, normal, albedo);

    for (uint i = 0u; i < MAX_LIGHTS; i++)
        L0 += PointLight(uPointLights[i], V, F0mix, normal, albedo);

    vec3 color = AMBIENT * albedo * material.AO + L0;

    FragColor = vec4(color, 1);
}
