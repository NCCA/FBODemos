#version 330 core
#pragma optionNV(fastmath on)
#pragma optionNV(fastprecision on)
#pragma optionNV(ifcvt all)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)

/// @brief the vertex passed in
layout (location=0) in vec3 inVert;
layout (location=1) in vec3 inNormal;
layout (location=2) in vec2 inUV;

layout(std140) uniform TransformUBO
{
uniform mat4 MVP;
uniform mat4 normalMatrix;
uniform mat4 M;
uniform mat4 MV;
}tx;
/// create an interface block for ease
out VertexData
{
  vec3 fragPos;
  vec2 uv;
  vec3 normal;
  vec3 fragVS;
}vertexOut;

void main()
{

  vertexOut.fragPos = vec3(tx.M * vec4(inVert,1.0));
  vertexOut.fragVS  = vec3(tx.MV* vec4(inVert,1.0));

  vertexOut.uv=inUV;
  vertexOut.normal=mat3(tx.normalMatrix)*inNormal;
  gl_Position = tx.MVP*vec4(inVert,1.0);
}
