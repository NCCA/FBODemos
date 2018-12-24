#version 330 core
#pragma optionNV(fastmath on)
#pragma optionNV(fastprecision on)
#pragma optionNV(ifcvt all)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)

out float fragColour;


uniform sampler2D positionSampler;
uniform sampler2D normalSampler;
uniform sampler2D texNoise;

uniform vec3 samples[64];
int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;
uniform vec2 screenResolution;
in vec2 uv;
uniform mat4 projection;


void main()
{
  vec2 noiseScale = vec2(screenResolution.x/4.0, screenResolution.y/4.0);
  // get input for SSAO algorithm
  vec3 fragPos = texture(positionSampler, uv).xyz;
  vec3 normal = normalize(texture(normalSampler, uv).rgb);
  vec3 randomVec = normalize(texture(texNoise, uv * noiseScale).xyz);
  // create TBN change-of-basis matrix: from tangent-space to view-space
  vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
  vec3 bitangent = cross(normal, tangent);
  mat3 TBN = mat3(tangent, bitangent, normal);
  // iterate over the sample kernel and calculate occlusion factor
  float occlusion = 0.0;
  for(int i = 0; i < kernelSize; ++i)
  {
      // get sample position
      vec3 sample = TBN * samples[i]; // from tangent to view-space
      sample = fragPos + sample * radius;

      // project sample position (to sample texture) (to get position on screen/texture)
      vec4 offset = vec4(sample, 1.0);
      offset = projection * offset; // from view to clip-space
      offset.xyz /= offset.w; // perspective divide
      offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0

      // get sample depth
      float sampleDepth = texture(positionSampler, offset.xy).z; // get depth value of kernel sample

      // range check & accumulate
      float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
      occlusion += (sampleDepth >= sample.z + bias ? 1.0 : 0.0) * rangeCheck;
  }
  occlusion = 1.0 - (occlusion / kernelSize);

  fragColour = occlusion;

}
