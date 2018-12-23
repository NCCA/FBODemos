#version 330 core
/// @brief the vertex passed in
layout (location=0) in vec3 inVert;

uniform samplerBuffer TBO;
out vec4 colour;
mat4 getMatrix(int offset)
{
    return (mat4(texelFetch(TBO, offset),
                 texelFetch(TBO, offset + 1),
                 texelFetch(TBO, offset + 2),
                 texelFetch(TBO, offset + 3)));
}
void main()
{
 const int size=5;
 mat4 MVP=getMatrix(gl_InstanceID*size);
 colour=texelFetch(TBO,gl_InstanceID*size+4);
 gl_Position = MVP*vec4(inVert,1.0);
}
