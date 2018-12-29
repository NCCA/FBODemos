#version 330 core
#pragma optionNV(fastmath on)
#pragma optionNV(fastprecision on)
#pragma optionNV(ifcvt all)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)

/// @brief the vertex passed in
layout (location = 0) in vec3 inVert;
/// @brief the in uv
layout (location = 1) in vec2 inUV;
/// @brief the normal passed in
layout (location = 2) in vec3 inNormal;
// Bone data
layout (location=3) in ivec4 BoneIDs;
layout (location=4) in vec4  Weights;


/// create an interface block for ease
out VertexData
{
  vec3 fragPos;
  vec2 uv;
  vec3 normal;
  vec3 fragVS;
  float texID;
}vertexOut;


layout(std140) uniform TransformUBO
{
   mat4 MVP;
   mat4 M;
   mat4 MV;
   int step;
}tx;

uniform samplerBuffer TBO;





mat4 getMatrix(int offset)
{
    return (mat4(texelFetch(TBO, offset),
                 texelFetch(TBO, offset + 1),
                 texelFetch(TBO, offset + 2),
                 texelFetch(TBO, offset + 3)));
}



void main()
{
   const int size=64;
   mat4 gBones[16];
   for(int i=0; i<16; ++i)
   {
     gBones[i]=getMatrix((tx.step*size)+(i*4));
   }
	 mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];
   BoneTransform     += gBones[BoneIDs[1]] * Weights[1];
   BoneTransform     += gBones[BoneIDs[2]] * Weights[2];
   BoneTransform     += gBones[BoneIDs[3]] * Weights[3];
   vec4 pos   = BoneTransform*vec4(inVert, 1.0);

   vertexOut.fragPos = vec3(tx.M * pos);
   vertexOut.fragVS  = vec3(tx.MV* pos);
   vertexOut.uv=inUV;
   vertexOut.normal=inNormal;
   vertexOut.texID=3;
   gl_Position = tx.MVP*pos;


}
