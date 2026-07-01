#version 460 core

layout(location = 1) out vec4 Normal;

in vec3 vNormal;
in vec3 vTangent;
in vec2 vTexCoords;

uniform sampler2D uNormals;
uniform sampler2D uColor;

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
    if (texture(uColor, vTexCoords).a < 0.5) discard;

	vec3 normal = normalize(vNormal);
	vec3 tangent = normalize(vTangent);
	tangent = normalize(tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(normal, tangent);
	mat3 TBN = mat3(tangent, bitangent, normal);

	vec3 normalMap = texture(uNormals, vTexCoords).rgb * 2.0 - 1.0;

	vec3 N = normalize(TBN * normalMap);

	Normal = vec4(Encode(N), 0.0, 1.0);
}