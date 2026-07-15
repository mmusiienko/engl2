#version 460 core
layout(quads, fractional_even_spacing, ccw) in;

in vec3 tcPos[];
in patch int tcLod;

uniform mat4 uModel;
uniform mat4 uViewProjection;
out vec2 vPos;

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

     float h = dot(heightSample, uHeightWeights);

    return h;
}


void main()
{
    vec3 p0 = mix(tcPos[0], tcPos[1], gl_TessCoord.x);
    vec3 p1 = mix(tcPos[3], tcPos[2], gl_TessCoord.x);
    vec3 pos = mix(p0, p1, gl_TessCoord.y);

    vec4 worldPos = vec4(pos, 1.0);
    vPos = worldPos.xz;

    float val = sampleHeight(vPos);
    worldPos.y += val;

    gl_Position = uViewProjection * worldPos;
}