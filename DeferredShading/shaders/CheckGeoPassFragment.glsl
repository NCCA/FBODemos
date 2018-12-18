#version 330 core
layout (location = 0) out vec3 position;
layout (location = 1) out vec4 normal;
layout (location = 2) out vec4 albedoSpec;
layout (location = 3) out vec3 positionVS;

uniform sampler2D normalMapSampler;

in VertexData
{
  vec3 fragPos;
  vec2 uv;
  vec3 normal;
  vec3 fragVS;
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


vec3 getNormalFromMap(in vec3 WorldPos,in vec3 Normal,in vec2 uv)
{
    vec3 tangentNormal = texture(normalMapSampler, uv).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(uv);
    vec2 st2 = dFdy(uv);

    vec3 N   = normalize(Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}


void main()
{

  // store the fragment position vector in the first gbuffer texture
    position = vertexIn.fragPos;
    // also store the per-fragment normals into the gbuffer
//    normal.rgb = normalize(vertexIn.normal);
    normal.rgb = getNormalFromMap(vertexIn.fragPos,vertexIn.normal,vertexIn.uv);
    positionVS=vertexIn.fragVS;

    // and the diffuse per-fragment color
    albedoSpec = checker( vertexIn.uv);
    //roughness is w
    normal.w=albedoSpec.a;

}
