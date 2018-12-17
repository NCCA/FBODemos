#version 330 core
out float fragColour;

uniform sampler2D ssaoInput;
uniform vec2 screenResolution;
const float fourSquared=4.0*4.0;
void main()
{
  vec2  uv=vec2(gl_FragCoord.x/screenResolution.x,gl_FragCoord.y/screenResolution.y);


    vec2 texelSize = 1.0 / vec2(textureSize(ssaoInput, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssaoInput, uv + offset).r;
        }
    }
    fragColour = result / (fourSquared);
}
