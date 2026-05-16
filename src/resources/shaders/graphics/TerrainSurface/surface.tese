#version 460 core
layout(quads, fractional_even_spacing, ccw) in;

in vec3 tcPos[];
in patch int tcLod;

uniform mat4 uModel;
uniform mat4 uViewProjection;
uniform vec3 uCamPos;
uniform mat4 uShadowMapViewProjection;
out vec4 vShadowPos;
out vec3 vFragPos;
out vec2 vPos;
out flat int vLod;

uniform sampler2D uTerrainInfo;
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
    return h ;
}

out float vHeight;

void main()
{
    vec3 p0 = mix(tcPos[0], tcPos[1], gl_TessCoord.x);
    vec3 p1 = mix(tcPos[3], tcPos[2], gl_TessCoord.x);
    vec3 pos = mix(p0, p1, gl_TessCoord.y);

    vec4 worldPos = vec4(pos, 1.0);
    vPos = worldPos.xz;
    vLod = tcLod;

    float val = sampleHeight(vPos);
    worldPos.y += val;

    vHeight = worldPos.y;

    vFragPos = worldPos.xyz;

    vShadowPos = uShadowMapViewProjection * worldPos;


    gl_Position = uViewProjection * worldPos;
}