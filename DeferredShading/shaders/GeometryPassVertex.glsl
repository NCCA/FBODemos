#version 330 core
// GeometryPassFragment.glsl
/// @brief the vertex passed in
layout (location=0) in vec3 inVert;
layout (location=1) in vec3 inNormal;
layout (location=2) in vec2 inUV;

uniform samplerBuffer TBO;

/// create an interface block for ease
out VertexData
{
  vec3 fragPos;
  vec2 uv;
  vec3 normal;
  vec3 fragVS;
}vertexOut;


// MVP          mat4 (offset, offset+1, offset+2, offset+3)
// normalMatrix mat4(offset+4, offset+5, offset+6,offset+7)
// M mat4(offset+8, offset+9, offset+10,offset+11)
// MV mat4(offset+12, offset+13, offset+15,offset+15)


mat4 getMatrix(int offset)
{
    return (mat4(texelFetch(TBO, offset),
                 texelFetch(TBO, offset + 1),
                 texelFetch(TBO, offset + 2),
                 texelFetch(TBO, offset + 3)));
}


void main()
{
  int ID=0;
  const int size=16;
  mat4 MVP=         getMatrix((gl_InstanceID*size)+0);
  mat4 normalMatrix=getMatrix((gl_InstanceID*size)+4);
  mat4 M=           getMatrix((gl_InstanceID*size)+8);
  mat4 MV=          getMatrix((gl_InstanceID*size)+12);

  vertexOut.fragPos = vec3(M * vec4(inVert,1.0));
  vertexOut.fragVS  = vec3(MV* vec4(inVert,1.0));
  vertexOut.uv=inUV;
  vertexOut.normal=mat3(normalMatrix)*inNormal;
  gl_Position = MVP*vec4(inVert,1.0);
}
