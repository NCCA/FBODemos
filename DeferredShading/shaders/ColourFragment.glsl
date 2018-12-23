#version 330 core
layout (location=0) out vec4 fragColour;
layout (location=1) out vec4 brightness;
//uniform vec4 colour;
in vec4 colour;

uniform vec2 screenResolution;
void main()
{
  vec2  uv=vec2(gl_FragCoord.x/screenResolution.x,gl_FragCoord.y/screenResolution.y);
  fragColour=colour;

  float bright = dot(fragColour.rgb, vec3(0.2126, 0.7152, 0.0722));
  if(bright > 1.0)
      brightness = vec4(fragColour.rgb, 1.0);
  else
      brightness = vec4(0.0, 0.0, 0.0, 1.0);
}
