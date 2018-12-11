#version 330 core

#define MAX_BLUR 20.0

uniform   float uFocusDistance;
uniform   float uBlurCoefficient;
uniform   float uPPM;
uniform   vec2  uDepthRange;
uniform   vec2 uResolution;
uniform int pass;
uniform vec2 uTexelOffset;
uniform sampler2D uColor;
uniform sampler2D uDepth;
in vec2 uv;
layout (location=0)out vec4 fragColor;

void main()
{

    ivec2 fragCoord = ivec2(gl_FragCoord.xy);
    ivec2 resolution = ivec2(uResolution) - 1;

    // Convert to linear depth
    float ndc = 2.0 * texelFetch(uDepth, fragCoord, 0).r - 1.0;
//    float ndc = 2.0 * texture(uDepth, uv).r - 1.0;

    float depth = -(2.0 * uDepthRange.y * uDepthRange.x) / (ndc * (uDepthRange.y - uDepthRange.x) - uDepthRange.y - uDepthRange.x);
    float deltaDepth = abs(uFocusDistance - depth);

    // Blur more quickly in the foreground.
    float xdd = depth < uFocusDistance ? abs(uFocusDistance - deltaDepth) : abs(uFocusDistance + deltaDepth);
    float blurRadius = min(floor(uBlurCoefficient * (deltaDepth / xdd) * uPPM), MAX_BLUR);

    vec4 color = vec4(0.0);
    if (blurRadius > 1.0)
    {
        float halfBlur = blurRadius * 0.5;

        float count = 0.0;

        for (float i = 0.0; i <= MAX_BLUR; ++i)
        {
            if (i > blurRadius)
            {
                break;
            }

            // texelFetch outside texture gives vec4(0.0) (undefined in ES 3)
            ivec2 sampleCoord = clamp(fragCoord + ivec2(((i - halfBlur) * uTexelOffset)), ivec2(0), resolution);
            color += texelFetch(uColor, sampleCoord, 0);
            //color += texture(uColor,uv);
            ++count;
        }

        color /= count;
        //color=vec4(0,1,0,0);
    }
    else
    {
        //color = vec4(1,0,0,0);
     color=texelFetch(uColor, fragCoord, 0);
    }

    fragColor = color;
}
