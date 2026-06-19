#version 460 core

out vec4 FragColor;
in vec2 vTexCoords;
in vec3 vFragPos;
in vec3 vNormal;

uniform vec4 uColor;

struct UniformPointLight
{
	vec3 Position;
	vec3 Color;
	float Intensity;
};

struct UniformDirectionalLight
{
	vec3 Direction;
	vec3 Color;
};

//uniform UniformPointLight uLights[MAX_LIGHTS];
uniform UniformDirectionalLight uDirectionalLight;
	
uniform vec3 uCameraPos;

uniform uvec2 uResolution;

uniform sampler2D uShadowMap;

vec3 DirectionalLight(UniformDirectionalLight light, vec3 objColor, vec3 viewDir, vec3 normal)
{
	float diff = max(dot(normal, light.Direction), 0.0);
	vec3 diffuse = light.Color * (diff * objColor);

	vec3 reflectVec = reflect(-light.Direction, normal);
	float spec = pow(max(dot(reflectVec, viewDir), 0.0), 32.0);
	vec3 specular = light.Color * (spec * objColor);

	return diffuse;
}

float Shadow(vec3 normal, UniformDirectionalLight light)
{
    vec4 s = texture(uShadowMap, gl_FragCoord.xy / vec2(uResolution));
    return s.r;
}

void main()
{
	vec3 color = uColor.rgb;
	vec3 normal = normalize(vNormal);

	vec3 viewDir = normalize(uCameraPos - vFragPos);
	float shadow = Shadow(normal, uDirectionalLight);
	color = shadow * DirectionalLight(uDirectionalLight, color, viewDir, normal);
	color += uColor.rgb * 0.3;

	FragColor = vec4(color, 1.0);
}