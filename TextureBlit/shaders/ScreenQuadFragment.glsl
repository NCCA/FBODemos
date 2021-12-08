#version 330 core

layout(location=0)out vec4 fragColour;

uniform sampler2D tex;
uniform int width;
uniform int height;

void main()
{
  vec2  uv=vec2(gl_FragCoord.x/width,gl_FragCoord.y/height);
  fragColour=texture(tex,uv);  
  
}
