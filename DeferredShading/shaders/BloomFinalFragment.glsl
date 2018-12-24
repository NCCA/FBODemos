#version 330 core
#pragma optionNV(fastmath on)
#pragma optionNV(fastprecision on)
#pragma optionNV(ifcvt all)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)

out vec4 fragColour;
uniform sampler2D scene;
uniform sampler2D bloomBlur;

layout( std140) uniform BloomParam
{
 bool bloom;
 float exposure;
 float gamma;
}tbo;

in vec2 uv;
void main()
{

    vec3 hdrColor = texture(scene, uv).rgb;
    vec3 bloomColor = texture(bloomBlur, uv).rgb;
    if(tbo.bloom)
        hdrColor += bloomColor; // additive blending
    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * tbo.exposure);
    // also gamma correct while we're at it
    result = pow(result, vec3(1.0 / tbo.gamma));
    fragColour = vec4(result, 1.0);
}
