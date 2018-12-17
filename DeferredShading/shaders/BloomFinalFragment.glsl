#version 330 core
out vec4 fragColour;

uniform vec2 screenResolution;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform bool bloom;
uniform float exposure;
uniform float gamma=2.2;

void main()
{
  vec2  uv=vec2(gl_FragCoord.x/screenResolution.x,gl_FragCoord.y/screenResolution.y);

    vec3 hdrColor = texture(scene, uv).rgb;
    vec3 bloomColor = texture(bloomBlur, uv).rgb;
    if(bloom)
        hdrColor += bloomColor; // additive blending
    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    // also gamma correct while we're at it
    result = pow(result, vec3(1.0 / gamma));
    fragColour = vec4(result, 1.0);
//    fragColour=vec4(hdrColor,1);
}
