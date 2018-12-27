#version 330 core
/// @brief the vertex passed in
layout (location = 0) in vec3 inVert;
/// @brief the in uv
layout (location = 1) in vec2 inUV;
/// @brief the normal passed in
layout (location = 2) in vec3 inNormal;
// Bone data
layout (location=3) in ivec4 BoneIDs;
layout (location=4) in vec4  Weights;

const int MAX_BONES = 16;

/// create an interface block for ease
out VertexData
{
  vec3 fragPos;
  vec2 uv;
  vec3 normal;
  vec3 fragVS;
  float texID;
}vertexOut;

uniform mat4 MVP;
uniform mat4 M;
uniform mat4 MV;

uniform mat4 gBones[MAX_BONES];

void main()
{
	 mat4 BoneTransform = gBones[BoneIDs[0]] * Weights[0];
   BoneTransform     += gBones[BoneIDs[1]] * Weights[1];
   BoneTransform     += gBones[BoneIDs[2]] * Weights[2];
   BoneTransform     += gBones[BoneIDs[3]] * Weights[3];
   vec4 pos   = BoneTransform*vec4(inVert, 1.0);

   vertexOut.fragPos = vec3(M * pos);
   vertexOut.fragVS  = vec3(MV* pos);
   vertexOut.uv=inUV;
   vertexOut.normal=inNormal;
   vertexOut.texID=3;
   gl_Position = MVP*pos;


}
