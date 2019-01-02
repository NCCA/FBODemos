#version 410 core

// this is a pointer to the current 2D texture object
uniform sampler2DMS tex;
// the vertex UV
in vec2 vertUV;
layout (location=0) out vec4 outColour;
uniform int numSamples;


subroutine vec4 sampling( in vec2 texcoord);
subroutine uniform sampling samplingSelection;

subroutine (sampling)
vec4 getMultiSampleTexture(in vec2 texcoord)
{
  // as we are using texel fetch we need integer values to sample
  // so find texture size and convert to int offsets
  ivec2 pos = ivec2(texcoord * vec2(textureSize(tex)));
  vec4 texel = vec4(0);
  // now loop for samples and accumulate
  for (int i = 0; i < numSamples; ++i)
  {
    vec4 samples = texelFetch(tex, pos, i);
    texel += samples;
  }
  // return the average value (could use other algorithms here)
  return texel / float(numSamples);
}


subroutine (sampling)
vec4 getMultiSampleTexture2(in vec2 texcoord)
{
  // as we are using texel fetch we need integer values to sample
  // so find texture size and convert to int offsets
  ivec2 pos = ivec2(texcoord * vec2(textureSize(tex)));
  vec4 texel = vec4(0);
  // now loop for samples and accumulate
  for (int i = 0; i < numSamples; ++i)
  {
    texel=max(texel,texelFetch(tex, pos, i));
  }
  return texel ;
}


void main ()
{
 // set the fragment colour to the current texture
 outColour = samplingSelection(vertUV);
}
