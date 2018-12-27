#version 330 core
/// @brief the vertex passed in
layout(location=0)in vec4 inVert;
/// @brief the colour of the point
layout(location=1)in vec4 inColour;
uniform mat4 MVP;

out vec4 pointColour;

void main()
{
  gl_PointSize=inVert.w;

  pointColour=inColour;
  gl_Position = MVP * vec4(inVert.xyz,1);
}
