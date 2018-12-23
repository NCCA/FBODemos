#version 330 core
out vec4 fragColour;


uniform sampler2D image;

uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
uniform vec2 screenResolution;
in vec2 uv;
void main()
{
 // vec2  uv=vec2(gl_FragCoord.x/screenResolution.x,gl_FragCoord.y/screenResolution.y);

  vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel
    vec3 result = texture(image, uv).rgb * weight[0]; // current fragment's contribution
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, uv + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, uv - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, uv + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(image, uv - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    fragColour = vec4(result, 1.0);
}
