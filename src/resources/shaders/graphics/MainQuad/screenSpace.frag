#version 460

in vec2 vTexCoords;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform sampler2D uSkyTexture;
uniform sampler2D uDepth;

uniform float uNear;
uniform float uFar;
uniform float uCloudRes  = 4.0;
struct UniformDirectionalLight { vec3 Direction; vec3 Color; };
uniform UniformDirectionalLight uDirectionalLight;

const vec3  gamma    = vec3(2.2);
const vec3  invgamma = 1.0 / gamma;
const float exposure = 1;

float linDepth(float depth)
{
    float z = depth * 2.0 - 1.0;
    return (2.0 * uNear * uFar) / (uFar + uNear - z * (uFar - uNear));
}

vec3 applyFog( vec3  col, 
               float t )
{
    if (t == uFar) return col;
    float fogAmount = 1.0 - exp(-t/1e4);
    vec3  fogColor  = vec3(0.5,0.6,0.7);
    return mix( col, fogColor, fogAmount * 0.1 );
}

const vec3 bgColor = vec3(0.2, 0.2, 0.4);
void main()
{
    vec4  frameColor   = texture(uTexture, vTexCoords);
    float frameDepth   = linDepth(texture(uDepth, vTexCoords).r);

    vec4  sky = texture(uSkyTexture, vTexCoords);

    vec3 color = applyFog(frameColor.rgb, frameDepth) * sky.a + sky.rgb;

    vec3 tonemapped      = vec3(1.0) - exp(-color * exposure);
    vec3 gammaCorrected  = pow(tonemapped, invgamma);

    FragColor = vec4(gammaCorrected, 1.0);
}
