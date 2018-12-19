#version 330 core
/// @brief the vertex passed in
layout (location=0) in vec3 inVert;
layout (location=1) in vec3 inNormal;
layout (location=2) in vec2 inUV;


//uniform mat4 MV;
//uniform mat4 MVP;
//uniform mat3 normalMatrix;
//uniform mat4 M;

layout( std140) uniform TransformUBO
{
  mat4 MVP;
  mat4 normalMatrix;
  mat4 M;
  mat4 MV;
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
