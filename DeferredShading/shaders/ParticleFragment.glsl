#version 330 core
#pragma optionNV(fastmath on)
#pragma optionNV(fastprecision on)
#pragma optionNV(ifcvt all)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)

layout (location=0) out vec4 fragColour;
layout (location=1) out vec4 brightness;
in vec4 pointColour;

void main()
{
  fragColour=pointColour;
  float bright = dot(fragColour.rgb, vec3(0.2126, 0.7152, 0.0722));
  if(bright > 1.0)
      brightness = vec4(fragColour.rgb, 1.0);
  else
      brightness = vec4(0.0, 0.0, 0.0, 1.0);
}
