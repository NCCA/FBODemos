#version 330
#pragma optionNV(fastmath on)
#pragma optionNV(fastprecision on)
#pragma optionNV(ifcvt all)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)

uniform sampler2D Texture;
in vec2 Frag_UV;
in vec4 Frag_Color;
layout (location=0) out vec4 Out_Color;
void main()
{
  Out_Color = Frag_Color * texture( Texture, Frag_UV.st);
}
