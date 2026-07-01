#version 460 core

uniform mat4 uInvView;
uniform mat4 uInvProjection;
uniform vec2 uResolution;
uniform vec3 uCameraPos;

uniform float uNear;
uniform float uFar;

uniform float uTime;

struct Color { vec3 Ambient; vec3 Diffuse; vec3 Specular; };
struct UniformDirectionalLight { vec3 Direction; Color Color; };
uniform UniformDirectionalLight uDirectionalLight = UniformDirectionalLight(normalize(vec3(1.0, -1.0, 0)), Color(vec3(0.1,0.076,0.05), vec3(0,0.8,0), vec3(0,0.8,0)));
uniform UniformDirectionalLight uDirectionalLight2 = UniformDirectionalLight(normalize(vec3(1.0, -1.0, 0)), Color(vec3(0.85,0.7,1), vec3(0,0.8,0), vec3(0,0.8,0)));

in vec2 vTexCoords;
out vec4 FragColor;

uniform vec3 uCloudScale = vec3(0.001);
uniform vec3 uDetailScale = vec3(1);
uniform vec4 uNoiseWeights = vec4(1);
uniform vec4 uDetailNoiseWeights = vec4(1);

uniform float uSkySpan = 10000.0;
uniform float uSkyHeightMin = 200.0;
uniform float uSkyHeightMax = 300.0;

uniform uint uNumSteps = 64;
uniform uint uNumStepsLight = 32;

uniform float uDensityOffset = 0.2;
uniform float uDensityScale = 1.0;
uniform float uErosionScale = 1.0;

uniform vec3 uDirection = vec3(0);
uniform float uSpeed = 1.0;

uniform sampler2D uDepth;
uniform sampler2D uColor;
uniform sampler3D uWorley1;
uniform sampler3D uWorley2;

uniform float uGlobalCoverage = 0.5;
uniform float uGlobalOpacity = 0.3;

uniform float uDarknessThreshold = 0.0;

uniform float uLightAbsorptionSun = 1.0;
uniform float uLightAbsorptionCloud = 1.0;

uniform float uPhaseVal = 1.0;

uniform float uExposure = 1.5;
uniform float uInvGamma = 1 / 2.2;


float linDepth()
{
	float depth = texture(uDepth, vTexCoords).r;
    float z = depth * 2.0 - 1.0;
    return (2.0 * uNear * uFar) / (uFar + uNear - z * (uFar - uNear));
}

float remap(float val, float mn1, float mx1, float mn2, float mx2)
{
	//[mn1 mx1] -> [0 mx1-mn1] -> [0 1] -> [0 mx2 - mn2] -> [mn2 mx2] 
	return (val - mn1) / (mx1 - mn1) * (mx2 - mn2) + mn2;
}

float saturate(float val)
{
	return clamp(val, 0, 1);
}

float lerp(float mn, float mx, float t)
{
	return mn * (1 - t) + t * mx; 
}

float random(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
}

float sphereSdf(vec3 center, float r)
{
	return length(center) - r;
}

float boxSdf(vec3 p, vec3 b)
{
  vec3 q = abs(p) - b;
  return length(max(q, 0.0)) + min(max(q.x, max(q.y,q.z)), 0.0);
}

const uint ITERATIONS = 10;

float map(vec3 rayPos)
{
	float dist = 10000;
	
	for (int i = 0; i < ITERATIONS; i++)
	{
		float spacing = i * 100;
		float w = 10;
		dist = min(
			sphereSdf(rayPos - vec3(spacing, 0, spacing), w),
			dist
		);
	}

	return max(boxSdf(vec3(0.0) - rayPos, vec3(50.0, 50.0, 50.0)), boxSdf(vec3(25.0) - rayPos, vec3(50.0, 50.0, 10.0)));
}

void main()
{
	vec2 ndc = gl_FragCoord.xy / uResolution * 2.0 - 1.0;

	vec3 boundsMin = vec3(uCameraPos.x - uSkySpan, uSkyHeightMin, uCameraPos.z - uSkySpan);
	vec3 boundsMax = vec3(uCameraPos.x + uSkySpan, uSkyHeightMax, uCameraPos.z + uSkySpan);

	vec4 clipFar  = vec4(ndc, -1.0, 1.0);

	vec4 viewFar  = uInvProjection * clipFar;  
	viewFar /= viewFar.w;

	vec4 worldFar  = uInvView * viewFar;

	vec3 rayOrigin = uCameraPos.xyz;             
	vec3 rayDir = normalize(worldFar.xyz - rayOrigin);

	float dist = 0.0;
	vec3 pos = rayOrigin;
	float stepSize = 0.0;
	bool hit = false;
	float lim = linDepth();
	int i = 0;
	for (; i < 1024; i++)
	{
		stepSize = map(pos);

		if (stepSize < 0.001)
		{
			hit = true;
			break;
		}

		pos += stepSize * rayDir;
		dist += stepSize;

		if (dist > lim)
		{
			break;
		}
	}

	if (!hit)
	{
		discard;
	}

	vec3 color = vec3(1, 0.0, 0.0);

	vec3 exposureToneMapped = vec3(1.0) - exp(-color * uExposure);
	vec3 gammaCorrected = pow(exposureToneMapped, vec3(1 / 2.2));

	FragColor = vec4(gammaCorrected, 1);
}
