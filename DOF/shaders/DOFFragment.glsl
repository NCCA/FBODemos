#version 330 core
// see http://developer.download.nvidia.com/books/HTML/gpugems/gpugems_ch23.html
// this example based on https://github.com/tsherif/webgl2examples
#define MAX_BLUR 20.0

uniform   float focusDistance;
uniform   float blurCoefficient;
uniform   float PPM;
uniform   vec2  depthRange;
uniform   vec2 screenResolution;
uniform int pass;
uniform vec2 uTexelOffset;
uniform sampler2D colourSampler;
uniform sampler2D depthSampler;
layout (location=0)out vec4 fragColor;

void main()
{
    // get location of fragment
    ivec2 fragCoord = ivec2(gl_FragCoord.xy);
    ivec2 resolution = ivec2(screenResolution) - 1;

    // Convert to linear depth
    float ndc = 2.0 * texelFetch(depthSampler, fragCoord, 0).r - 1.0;

    float depth = -(2.0 * depthRange.y * depthRange.x) / (ndc * (depthRange.y - depthRange.x) - depthRange.y - depthRange.x);
    float deltaDepth = abs(focusDistance - depth);

    // Blur more quickly in the foreground.
    float xdd = depth < focusDistance ? abs(focusDistance - deltaDepth) : abs(focusDistance + deltaDepth);
    float blurRadius = min(floor(blurCoefficient * (deltaDepth / xdd) * PPM), MAX_BLUR);
    //  so we sample around the pixel and blur.
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
            color += texelFetch(colourSampler, sampleCoord, 0);
            ++count;
        }

        color /= count;
    }
    // other wise we are in the focal point so just use un blurred pixed
    // this does produce some slightly sharp edges.
    else
    {
     color=texelFetch(colourSampler, fragCoord, 0);
    }

    fragColor = color;
}
