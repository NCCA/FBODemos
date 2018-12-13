#version 330 core
layout (location = 0) out vec3 position;
layout (location = 1) out vec3 normal;
layout (location = 2) out vec4 albedoSpec;

in VertexData
{
  vec3 fragPos;
  vec2 uv;
  vec3 normal;
}vertexIn;

uniform sampler2D albedoSampler;
uniform sampler2D specularSampler;


void main()
{

  // store the fragment position vector in the first gbuffer texture
    position = vertexIn.fragPos;
    // also store the per-fragment normals into the gbuffer
    normal = normalize(vertexIn.normal);
    // and the diffuse per-fragment color
    albedoSpec.rgb = texture(albedoSampler, vertexIn.uv).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    albedoSpec.a = texture(specularSampler,  vertexIn.uv).r;

}
