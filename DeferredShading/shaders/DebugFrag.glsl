#version 330 core
#pragma optionNV(fastmath on)
#pragma optionNV(fastprecision on)
#pragma optionNV(ifcvt all)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)

layout (location=0) out vec4 fragColour;
uniform sampler2D image;
uniform vec2 screenResolution;
in vec2 uv;
void main()
{
  //vec2  uv=vec2(gl_FragCoord.x/screenResolution.x,gl_FragCoord.y/screenResolution.y);
  fragColour=texture(image,uv);
}
