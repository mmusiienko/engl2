#version 460 core

layout (vertices=4) out;

in gl_PerVertex
{
	vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[];
} gl_in[gl_MaxPatchVertices];

in vec3 vPos[];
out vec3 tcPos[];
uniform vec3 uCameraPos;
out patch int tcLod;

float tessLevel(float dist)
{
    if (dist < 4000)  return 16.0;
    if (dist < 8000)  return 12.0;
    if (dist < 16000) return 8.0;
    return 4.0;
}

void main()
{
	tcPos[gl_InvocationID] = vPos[gl_InvocationID];

    if (gl_InvocationID == 0) 
    {
        // Edge midpoints
        vec3 e0mid = (vPos[0] + vPos[3]) * 0.5; // left edge
        vec3 e1mid = (vPos[0] + vPos[1]) * 0.5; // bottom edge
        vec3 e2mid = (vPos[1] + vPos[2]) * 0.5; // right edge
        vec3 e3mid = (vPos[2] + vPos[3]) * 0.5; // top edge
        vec3 center = (vPos[0] + vPos[1] + vPos[2] + vPos[3]) * 0.25;

        gl_TessLevelOuter[0] = tessLevel(length(uCameraPos.xz - e0mid.xz));
        gl_TessLevelOuter[1] = tessLevel(length(uCameraPos.xz - e1mid.xz));
        gl_TessLevelOuter[2] = tessLevel(length(uCameraPos.xz - e2mid.xz));
        gl_TessLevelOuter[3] = tessLevel(length(uCameraPos.xz - e3mid.xz));

        float inner = max(max(gl_TessLevelOuter[0], gl_TessLevelOuter[1]),
                          max(gl_TessLevelOuter[2], gl_TessLevelOuter[3]));
        gl_TessLevelInner[0] = inner;
        gl_TessLevelInner[1] = inner;
    }
}