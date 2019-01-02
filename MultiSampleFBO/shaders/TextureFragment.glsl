#version 330 core

// this is a pointer to the current 2D texture object
uniform sampler2DMS tex;
// the vertex UV
in vec2 vertUV;
layout (location=0) out vec4 outColour;
uniform int numSamples;
vec4 getMultiSampleTexture(in vec2 texcoord)
{
  ivec2 pos = ivec2(texcoord * vec2(textureSize(tex)));
  vec4 texel = vec4(0,0,0,0);
  for (int i = 0; i < numSamples; ++i)
  {
    vec4 sample = texelFetch(tex, pos, i);
    texel += sample;
  }

  return texel / float(numSamples);
}




void main ()
{
 // set the fragment colour to the current texture
 outColour = getMultiSampleTexture(vertUV);
}
