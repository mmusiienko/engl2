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

void main()
{
    vec3 p0 = mix(tcPos[0], tcPos[1], gl_TessCoord.x);
    vec3 p1 = mix(tcPos[3], tcPos[2], gl_TessCoord.x);
    vec3 pos = mix(p0, p1, gl_TessCoord.y);

    vec3 displacement = vec3(0.0);

    vec4 worldPos = vec4(pos, 1.0);

    for (int i = 0; i < tcLod; i++)
    {
        vec2 uv = worldPos.xz / uCascades[i].L * uCascades[i].Tiling;
        vec3 val = texture(uCascades[i].Displacement, uv).rgb;

        displacement += val * uCascades[i].InvTiling;
    }
    worldPos += vec4(displacement, 0.0);

    gl_Position = uViewProjection * worldPos;
}