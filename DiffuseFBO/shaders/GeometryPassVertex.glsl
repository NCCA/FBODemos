#version 410 core
layout (location = 0) in vec3 inPos;
layout (location=1) in vec3 inNormal;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 MVP;
uniform mat4 model;
uniform mat3 normalMatrix;

void main() 
{
    FragPos = vec3(model * vec4(inPos, 1.0));
    Normal = normalMatrix * inNormal; 
    gl_Position = MVP * vec4(inPos, 1.0);
}
