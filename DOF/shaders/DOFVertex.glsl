#version 330 core

layout(location=0) in vec4 inVert;
layout(location=1) in vec2 inUV;
out vec2 uv;
void main()
{
 uv=inUV;
 gl_Position = inVert;
}
