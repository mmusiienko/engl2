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

struct IntersectionResult
{
	bool intersect;
	float tNear;
	float tFar;
};

IntersectionResult intersectSkyBox(vec3 rayOrigin, vec3 rayDir, vec3 boundsMin, vec3 boundsMax)
{
	IntersectionResult res;

    vec3 invDir = 1.0 / rayDir;
    vec3 t1 = (boundsMin - rayOrigin) * invDir;
    vec3 t2 = (boundsMax - rayOrigin) * invDir;

    vec3 tmin = min(t1, t2);
    vec3 tmax = max(t1, t2);

    res.tNear = max(max(tmin.x, tmin.y), tmin.z);
	res.tFar  = min(min(tmax.x, tmax.y), tmax.z);

    res.intersect =	(res.tNear <= res.tFar && res.tFar > 0.0);

    return res;
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

float sampleDensity(vec3 pos)
{
	vec3 samplePos = 0.001 * (pos + uSpeed * uDirection * uTime);
	float density = 0;
	float detailDensity = 0;
	density = dot(texture(uWorley1, samplePos * uCloudScale).rgba, uNoiseWeights) - 0.5;
	detailDensity = dot(texture(uWorley2, samplePos * uCloudScale).rgba, uDetailNoiseWeights) * 0.02;
	return max(density - detailDensity, 0);
}

float sampleTransmittance(vec3 pos, vec3 boundsMin, vec3 boundsMax)
{
	float stepSize = 0.5 * (uSkyHeightMax - uSkyHeightMin) / uNumStepsLight;
	float totalDensity = 0;
	vec3 lightDir = uDirectionalLight.Direction;
	lightDir.y *= -1;
	for (uint i = 0; i < uNumStepsLight; i++)
	{
		pos += lightDir * stepSize;
		totalDensity += sampleDensity(pos) * stepSize;
	}

	return uDarknessThreshold + exp(-totalDensity * uLightAbsorptionSun) * (1 - uDarknessThreshold);
}

float random(vec2 p) {
    return fract(sin(dot(p, vec2(12.9898, 78.233))) * 43758.5453);
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

	IntersectionResult res = intersectSkyBox(rayOrigin, rayDir, boundsMin, boundsMax);

	if (!res.intersect)
	{
		discard;
	}
	else
	{
		float depth = linDepth();
		float near = max(res.tNear, 0.0);
		float distInside = res.tFar - near;
		float distToTravel = min(depth - near, distInside);
		float dist = 0.0;
		float stepSizeOrig = 11;
		float stepSize = stepSizeOrig;
		vec3 pos = rayOrigin + (near + random(gl_FragCoord.xy) * 20) * rayDir;
		float lightEnergy = 0;
		float transmittance = 1;
		float inc_rate = 0.004;
		int i = 0;
		while (dist < distToTravel)
		{
			stepSize = stepSizeOrig + (dist + near) * inc_rate;
			pos += stepSize * rayDir;
			dist += stepSize;

			float density = sampleDensity(pos);
			if (density > 0)
			{
				float lightTransmittance = sampleTransmittance(pos, boundsMin, boundsMax);
				lightEnergy += density * stepSize * lightTransmittance * transmittance * uPhaseVal;
				transmittance *= exp(-density * stepSize * uLightAbsorptionCloud);

				if (transmittance < 0.01) break;
			}
			i++;
		}

		vec3 cloudColor = uDirectionalLight.Color.Ambient * lightEnergy;

		vec3 color = texture(uColor, vTexCoords).rgb * transmittance + cloudColor;

		vec3 exposureToneMapped = vec3(1.0) - exp(-color * uExposure);
		vec3 gammaCorrected = pow(exposureToneMapped, vec3(1 / 2.2));

		FragColor = vec4(gammaCorrected, 1);
	}
}
