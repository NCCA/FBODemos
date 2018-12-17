#version 330 core
layout (location = 0) in vec3 inPos;

//out vec2 uv;

void main()
{
  //  uv = inUV;
    gl_Position = vec4(inPos, 1.0);
}
