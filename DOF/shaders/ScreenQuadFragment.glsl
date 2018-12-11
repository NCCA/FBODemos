#version 330 core

layout(location=0)out vec4 fragColour;
uniform sampler2D colour;
in vec2 uv;
void main()
{
  fragColour=texture(colour,uv);
}
