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
    vec4 noiseoffset = texture(uTerrainInfo, pos * uScale.x * baseScale) * 1;
    noiseoffset.b = 1.0;
    noiseoffset.r *= 0.2;
    vec4 heightSample = vec4(
        texture(uTerrainInfo, pos * uScale.x * baseScale * noiseoffset.g).r,
        texture(uTerrainInfo, pos * uScale.y * baseScale * noiseoffset.b).g,
        texture(uTerrainInfo, pos * uScale.z * baseScale * noiseoffset.a).b,
        texture(uTerrainInfo, pos * uScale.w * baseScale * noiseoffset.r).a
    );

    heightSample = vec4(pow(heightSample.r, 10.0), heightSample.g * 2, 0.5 * heightSample.b, heightSample.a);

    float h = min(dot(heightSample, uHeightWeights) - 500, 30.0);

    return h;
}
out float vHeight;

uniform float uTerrainResolution = 1.0;

vec3 computeNormal(vec2 pos, float h)
{
    float s = uTerrainResolution / vLod;

    vec2 pL = pos + vec2(-s, 0.0);
    vec2 pR = pos + vec2( s, 0.0);
    vec2 pD = pos + vec2(0.0, -s);
    vec2 pU = pos + vec2(0.0,  s);

    float hL = sampleHeight(pL);
    float hR = sampleHeight(pR);
    float hD = sampleHeight(pD);
    float hU = sampleHeight(pU);

    vec3 left  = vec3(-s, hL - h,  0.0);
    vec3 right = vec3( s, hR - h,  0.0);
    vec3 down  = vec3( 0.0, hD - h, -s);
    vec3 up    = vec3( 0.0, hU - h,  s);

    vec3 n =
          cross(up, right) +
          cross(right, down) +
          cross(down, left) +
          cross(left, up);

    return normalize(n);
}

out vec3 vNormal;

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

    vNormal = computeNormal(vPos, vHeight);

    gl_Position = uViewProjection * worldPos;
}