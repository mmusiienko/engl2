#version 460 core
out vec4 FragColor;

in vec3 vFragPos;
in vec2 vPos;
in flat int vLod;
in vec4 vShadowPos;

uniform vec3 uCameraPos;
uniform samplerCube uCubemap;
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

uniform Material material = Material(vec3(0.1, 0.8, 0.1), 0.0, .7, 1.0);

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

float Shadow()
{
    vec3 proj = (vShadowPos.xyz / vShadowPos.w) * 0.5 + 0.5;
    if (proj.z > 1.0) return 0.0;

    vec2  texelSize = 1.0 / textureSize(uShadowMap, 0);
    float shadow    = 0.0;
    float bias      = 0.01;

    for (int x = -1; x <= 1; ++x)
        for (int y = -1; y <= 1; ++y)
        {
            float d = texture(uShadowMap, proj.xy + vec2(x, y) * texelSize).r;
            shadow += proj.z - bias > d ? GAUSSIAN_3X3[(x + 1) * 3 + (y + 1)] : 0.0;
        }

    return shadow / 16.0;
}
const vec3 F0 = vec3(0.04);
const vec3 AMBIENT  = vec3(0.7);

uniform sampler2D uTerrainInfo;
uniform float uResolution = 1.0;
uniform vec4 uHeightWeights = vec4(100.0, 1.0, 0.1, 0.001);
uniform vec4 uScale = vec4(0.001);
const float baseScale = 0.001;

float sampleHeight(vec2 pos)
{
    vec4 heightSample = vec4(
        texture(uTerrainInfo, pos * uScale.x * baseScale).r,
        texture(uTerrainInfo, pos * uScale.y * baseScale).g,
        texture(uTerrainInfo, pos * uScale.z * baseScale).b,
        texture(uTerrainInfo, pos * uScale.w * baseScale).a
    ) - vec4(0.5);
    float h = dot(heightSample, uHeightWeights);
    
    return h;
}

const vec3 GRASS = vec3(0.1, 0.8, 0.1);
const vec3 SNOW = vec3(1.0);
const vec3 SAND = vec3(224.0 / 255.0, 198.0 / 255.0, 49.0 / 255.0);
const vec3 ROCK = vec3(0.03);

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
    float h = height + texture(uTerrainInfo, vPos * uScale.y * baseScale).b * 300.0;
    float sandToGrass = grad(h, 0.0, 200.0, 0.0, 1.0);
    float grassToRock = grad(h,   150.0, 300.0, 0.0, 1.0);
    float rockToSnow  = grad(h,  2300.0, 2500.0, 0.0, 1.0);

    vec3 col = SAND;
    col = mix(col, GRASS, sandToGrass);
    col = mix(col, ROCK,  grassToRock);
    col = mix(col, SNOW,  rockToSnow);
    return col;
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

vec3 computeNormal(vec2 pos) {
    float eps = uResolution /16;
    float hL = sampleHeight(vPos + vec2(-eps, 0));
    float hR = sampleHeight(vPos + vec2(eps, 0));
    float hD = sampleHeight(vPos + vec2(0, -eps));
    float hU = sampleHeight(vPos + vec2(0, eps));

    vec3 normal = normalize(vec3(hL - hR, 2.0 * eps, hD - hU));
    return normal;
}

in float vHeight;

void main()
{
    FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
