#version 460

in vec2 vTexCoords;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform sampler2D uSkyTexture;
uniform sampler2D uDepth;
uniform sampler2D uSkyDepth;

uniform float uNear;
uniform float uFar;
uniform float uCloudRes  = 4.0;

const vec3  gamma    = vec3(2.2);
const vec3  invgamma = 1.0 / gamma;
const float exposure = 1.0;

float linDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * uNear * uFar) / (uFar + uNear - z * (uFar - uNear));
}


void main()
{
    vec4  frameColor   = texture(uTexture, vTexCoords);

    vec4  sky          = texture(uSkyTexture, vTexCoords);

    vec3 color = frameColor.rgb * (sky.a)+ 
                     sky.rgb;

    vec3 tonemapped      = vec3(1.0) - exp(-color * exposure);
    vec3 gammaCorrected  = pow(tonemapped, invgamma);

    FragColor = vec4(gammaCorrected, 1.0);
}
