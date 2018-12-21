#version 330 core
layout (location=0) out vec3 position;
// We use the W channel for roughness
layout (location=1) out vec4 normal;
// albedoMetallic RGB == albedo A=metallic
layout (location=2) out vec4 albedoMetallic;
layout (location=3) out vec3 positionVS;

in VertexData
{
  vec3 fragPos;
  vec2 uv;
  vec3 normal;
  vec3 fragVS;
  float texID;
}vertexIn;


uniform sampler2DArray albedoSampler;
uniform sampler2DArray metallicSampler;
uniform sampler2DArray roughnessSampler;
uniform sampler2DArray normalMapSampler;

vec3 getNormalFromMap(in vec3 WorldPos,in vec3 Normal,in vec2 uv,in float layer)
{
    vec3 tangentNormal = texture(normalMapSampler, vec3(uv,layer)).xyz * 2.0 - 1.0;

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
  float layer=vertexIn.texID;
  // store the fragment position vector in the first gbuffer texture
  position = vertexIn.fragPos;
  positionVS=vertexIn.fragVS;
  normal.rgb = getNormalFromMap(vertexIn.fragPos,vertexIn.normal,vertexIn.uv,layer);
  normal.a=texture(roughnessSampler,  vec3(vertexIn.uv,layer)).r;
  albedoMetallic.rgb = texture(albedoSampler, vec3(vertexIn.uv,layer)).rgb;
  albedoMetallic.a = texture(metallicSampler,  vec3(vertexIn.uv,layer)).r;

}
