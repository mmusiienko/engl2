#version 460 core
out vec4 FragColor;

in vec3 vFragPos;
in vec2 vPos;
in flat int vLod;
in vec4 vShadowPos;

const int NCascades = 4;

struct CascadeInfo
{
    sampler2D Displacement;
    sampler2D Normal;
    uint N;
    float L;
    float Tiling;
    float InvTiling;
    float FoamScale;
    float FoamFlatSubtract;
};

uniform CascadeInfo uCascades[NCascades];
uniform vec3 uCameraPos;
uniform samplerCube uCubemap;
uniform sampler2D uFoamDetail;
uniform sampler2D uDepth;
uniform float uNear;
uniform float uFar;
uniform float uTime;
uniform uvec2 uResolution;

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

uniform Material material = Material(vec3(0.01, 0.05, 0.07) * 2 , 0.0, 0.08, 1.0);

float linDepth(float depth)
{    
    float z = depth * 2.0 - 1.0;
    return (2.0 * uNear * uFar) / (uFar + uNear - z * (uFar - uNear));
}

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

vec3 DirectionalLightLobe(UniformDirectionalLight light, vec3 V, vec3 F0mat,
                           vec3 N, vec3 albedo, float roughness, float foam)
{
    vec3  L     = light.Direction;
    vec3  H     = normalize(V + L);
    float NDotH = max(dot(N, H), 0.0);
    float NDotV = max(dot(N, V), 0.0);
    float NDotL = max(dot(N, L), 0.0);
    float HDotV = max(dot(H, V), 0.0);

    float ndf     = DistributionGGX(NDotH, roughness);
    float G       = GeometrySmith(NDotV, NDotL, roughness);
    vec3  fresnel = FresnelSchlick(HDotV, F0mat);

    vec3 kD      = (vec3(1.0) - fresnel) * (1.0 - material.Metallic);
    vec3 specular = (foam < 0.001) ? ((ndf * G * fresnel) / (4.0 * NDotV * NDotL + 0.0001)) : vec3(0);

    return (kD * albedo / PI + specular) * light.Color * NDotL;
}

vec3 DirectionalLight(UniformDirectionalLight light, vec3 V, vec3 F0mat,
                      vec3 N, vec3 albedo, float foam)
{
    vec3 sharp = DirectionalLightLobe(light, V, F0mat, N, albedo, material.Roughness, foam);
    vec3 broad  = DirectionalLightLobe(light, V, F0mat, N, albedo, material.Roughness + 0.30, foam);
    return sharp * 0.75 + broad * 0.25;
}

vec3 PointLight(UniformPointLight light, vec3 V, vec3 F0mat, vec3 N, vec3 albedo, float foam)
{
    UniformDirectionalLight dirLight;
    dirLight.Direction = normalize(light.Position - vFragPos);
    dirLight.Color     = light.Color;

    float dist        = length(light.Position - vFragPos);
    float attenuation = 1.0 / (dist * dist);

    return DirectionalLight(dirLight, V, F0mat, N, albedo, foam) * attenuation * light.Intensity;
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

const vec3 F0_WATER = vec3(0.02);
const vec3 AMBIENT  = vec3(0.03);

void main()
{
    vec2  totalSlope = vec2(0.0);
    float totalFoam  = 0.0;

    for (int i = 0; i < NCascades; i++)
    {
        vec2 uv    = vPos / uCascades[i].L * uCascades[i].Tiling;
        vec4 ninfo = texture(uCascades[i].Normal, uv);
        totalSlope += ninfo.rg;
        totalFoam  += ninfo.a;
    }

    vec3 normal = normalize(vec3(-totalSlope.x, 1.0, -totalSlope.y));
    
    vec3  foamDetail = texture(uFoamDetail, vFragPos.xz * 0.0002).rgb;
    float depth      = texture(uDepth, gl_FragCoord.xy / vec2(uResolution)).r;

    totalFoam += clamp(
        5.0 * max(foamDetail.r - max(linDepth(depth) - linDepth(gl_FragCoord.z) - 5.0, 0.0), 0.0),
        0.0, 1.0) * 0.3;

    vec3  V     = normalize(uCameraPos - vFragPos);
    float NdotV = max(dot(normal, V), 0.0);

    vec3 albedo = material.Albedo * NdotV;

    vec3 F0mix = mix(F0_WATER, albedo, material.Metallic);

    vec3 L0 = vec3(0.0);
    float shadow = (1 - Shadow());
    L0 += shadow * DirectionalLight(uDirectionalLight, V, F0mix, normal, albedo, totalFoam);

    for (uint i = 0u; i < MAX_LIGHTS; i++)
        L0 += PointLight(uPointLights[i], V, F0mix, normal, albedo, totalFoam);

    vec3 color = AMBIENT * albedo * material.AO + L0;

    vec3 reflFresnel     = FresnelSchlick(NdotV, F0_WATER);
    vec3 reflectDir      = reflect(-V, -normal);
    vec3 reflectionColor = texture(uCubemap, reflectDir).rgb;

    color += shadow * reflectionColor * (0.15 * reflFresnel);

    totalFoam = max(totalFoam, 0.0);

    color = mix(color, color + foamDetail, shadow * totalFoam);

    FragColor = vec4(color, 1-  0.003 * dot(reflectDir, normal));
}
