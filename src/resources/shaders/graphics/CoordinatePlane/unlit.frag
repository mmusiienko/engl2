#version 460 core

out vec4 FragColor;

uniform vec3 uCameraPos;
in vec3 vWorldPos;

uniform float cellScale = 10.0;
uniform float cellSize = 1.0;
uniform float minCellSizePx = 1.0;
vec4 color0 = vec4(0.0, 0.0, 0.0, 1.0);
vec4 color1 = vec4(0.0, 0.5, 0.0, 1.0);


float max2(vec2 v)
{
	return max(v.x, v.y);
}

vec2 satv(vec2 v)
{
	return clamp(v, 0.0, 1.0);
}

float calcLod(vec2 dudv, float cellSize)
{
	return max2(vec2(1.0) - abs(satv(mod(vWorldPos.xz, cellSize) / dudv) * 2.0 - vec2(1.0)));
}

float log10(float x)
{
	return log(x) / log(10.0);
}

void main()
{
	vec2 dvx = vec2(dFdx(vWorldPos.x), dFdy(vWorldPos.x));
	vec2 dvy = vec2(dFdx(vWorldPos.z), dFdy(vWorldPos.z));

	float lx = length(dvx);
	float ly = length(dvy);

	vec2 dudv = vec2(lx, ly);

	float l = length(dudv);

	float lod = max(0.0, log10(l * minCellSizePx / cellSize) + 1.0);
	float cellSize0 = cellSize * pow(10.0, floor(lod));
	float cellSize1 = cellSize0 * cellScale;
	float cellSize2 = cellSize1 * cellScale;

	dudv *= 4.0;

	float lod0 = calcLod(dudv, cellSize0);
	float lod1 = calcLod(dudv, cellSize1);
	float lod2 = calcLod(dudv, cellSize2);

	float fade = fract(lod);

	vec4 color;
	if (lod2 > 0.0)
	{
		color = color1;
		color.a *= lod2;
	} 
	else
	{
		if (lod1 > 0.0)
		{
			color = mix(color1, color0, fade);
			color.a *= lod1;
		} 
		else
		{
			color = color0;
			color.a *= lod1 * fade;
		}
	}
	float falloff = clamp(2.5 * max(uCameraPos.y, 300.0) / length(vWorldPos.xz - uCameraPos.xz), 0.0, 1.0);

	color.a = color.a * falloff;
	FragColor = color;
}