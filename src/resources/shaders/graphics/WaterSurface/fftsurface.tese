#version 460 core
layout(quads, fractional_even_spacing, ccw) in;

in vec3 tcPos[];
in patch int tcLod;

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

uniform mat4 uModel;
uniform mat4 uViewProjection;
uniform vec3 uCamPos;
uniform mat4 uShadowMapViewProjection;
out vec4 vShadowPos;
out vec3 vFragPos;
out vec2 vPos;
out flat int vLod;

void main()
{
    vec3 p0 = mix(tcPos[0], tcPos[1], gl_TessCoord.x);
    vec3 p1 = mix(tcPos[3], tcPos[2], gl_TessCoord.x);
    vec3 pos = mix(p0, p1, gl_TessCoord.y);

    vec3 displacement = vec3(0.0);

    vec4 worldPos = vec4(pos, 1.0);
    vPos = worldPos.xz;
    vLod = tcLod;

    for (int i = 0; i < vLod; i++)
    {
        vec2 uv = worldPos.xz / uCascades[i].L * uCascades[i].Tiling;
        vec3 val = texture(uCascades[i].Displacement, uv).rgb;

        displacement += val * uCascades[i].InvTiling;
    }
    worldPos += vec4(displacement, 0.0);

    vFragPos = worldPos.xyz;

    vShadowPos = uShadowMapViewProjection * worldPos;

    gl_Position = uViewProjection * worldPos;
}