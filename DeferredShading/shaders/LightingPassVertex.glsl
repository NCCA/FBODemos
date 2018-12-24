#version 330 core
#pragma optionNV(fastmath on)
#pragma optionNV(fastprecision on)
#pragma optionNV(ifcvt all)
#pragma optionNV(inline all)
#pragma optionNV(strict on)
#pragma optionNV(unroll all)

layout (location = 0) in vec3 inPos;

//out vec2 uv;

void main()
{
  //  uv = inUV;
    gl_Position = vec4(inPos, 1.0);
}
