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

void main()
{
	tcPos[gl_InvocationID] = vPos[gl_InvocationID];

    if (gl_InvocationID == 0) 
    {
        vec3 center = (vPos[0] + vPos[1] + vPos[2] + vPos[3]) * 0.25;
        float dist = length(uCameraPos - center);
        int lod = 1;
        
        if (dist < 1000)
        {
            lod = 4;
        } else if (dist < 2000)
        {
            lod = 3;
        } else if (dist < 4000)
        {
            lod = 2;
        }

        tcLod = lod;

        float subDiv = 4.0 * lod;

        gl_TessLevelInner[0] = subDiv;
        gl_TessLevelInner[1] = subDiv;
        gl_TessLevelOuter[0] = subDiv;
        gl_TessLevelOuter[1] = subDiv;
        gl_TessLevelOuter[2] = subDiv;
        gl_TessLevelOuter[3] = subDiv;
    }
}