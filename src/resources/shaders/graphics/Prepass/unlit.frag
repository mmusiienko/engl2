#version 460 core

layout(location = 1) out vec4 Normal;

in vec3 vNormal;

//https://knarkowicz.wordpress.com/2014/04/16/octahedron-normal-vector-encoding/
vec2 OctWrap(vec2 v)
{
    return (1.0 - abs(v.yx)) * sign(v.xy);
}

vec2 Encode(vec3 n)
{
    n /= (abs(n.x) + abs(n.y) + abs(n.z));
    n.xy = n.z >= 0.0 ? n.xy : OctWrap(n.xy);
    return n.xy * 0.5 + 0.5;
}

vec3 Decode(vec2 f)
{
    f = f * 2.0 - 1.0;
    // https://twitter.com/Stubbesaurus/status/937994790553227264
    vec3 n = vec3(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
    float t = clamp(-n.z, 0.0, 1.0);
    n.xy -= sign(n.xy) * t;
    return normalize(n);
}

void main()
{
	Normal = vec4(Encode(vNormal), 0.0, 1.0);
}