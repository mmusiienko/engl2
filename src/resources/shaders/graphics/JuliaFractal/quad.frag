#version 460 core

uniform mat4 uInvView;
uniform mat4 uInvProjection;
uniform uvec2 uResolution;

uniform double uCameraPosX;
uniform double uCameraPosY;

uniform float uTime;

uniform double uZoom;

uniform vec2 uWrap = vec2(0.5, 1);
uniform vec2 uOffset = vec2(0.5);

in vec2 vTexCoords;
out vec4 FragColor;

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

const uint ITERATIONS = 256;
const double width = 10.0;
const float camSpeed = 0.2;


void main()
{
	double height = width * (uResolution.y / float(uResolution.x));

	vec2 uv = (vTexCoords - uOffset) * uWrap;
	
	vec2 complexCameraSpace = uv * vec2(width, height);

	double currX = complexCameraSpace.x + uCameraPosX;
	double currY = complexCameraSpace.y + uCameraPosY;

	currX = uCameraPosX + (currX - uCameraPosX) * uZoom;
	currY = uCameraPosY + (currY - uCameraPosY) * uZoom;

	int i = 0;

	double zx = currX;
	double zy = currY;

	for (; i < ITERATIONS; i++)
	{
		double zx2 = zx * zx;
		double zy2 = zy * zy;
		if (zx2 + zy2 > 4) break;

		zy = 2.0 * zx * zy + uCameraPosY;
		zx = zx2 - zy2 + uCameraPosX;
	}

	float it = 1 - i / float(ITERATIONS);
	FragColor = vec4(it, it * it, 10 * it - 10.0 + it * it, 1);
}
