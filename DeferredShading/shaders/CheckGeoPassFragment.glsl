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

uniform float checkSize=10.0;
uniform vec4 colour1;
uniform vec4 colour2;

vec4 checker( vec2 uv )
{
  float v = floor( checkSize * uv.x ) +floor( checkSize * uv.y );
  if( mod( v, 2.0 ) < 1.0 )
     return colour2;
  else
     return colour1;

}

void main()
{

  // store the fragment position vector in the first gbuffer texture
    position = vertexIn.fragPos;
    // also store the per-fragment normals into the gbuffer
    normal = normalize(vertexIn.normal);
    // and the diffuse per-fragment color
    albedoSpec.rgb = checker( vertexIn.uv).rgb;
    // store specular intensity in gAlbedoSpec's alpha component
    albedoSpec.a = 0.0f;

}
