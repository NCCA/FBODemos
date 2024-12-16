#version 410 core
layout(location=0) out vec3 position;
layout(location=1) out vec3 normal;
layout(location=2) out vec3 albedo;

in vec3 FragPos;
in vec3 Normal;
uniform vec3 colour;
void main() 
{
    position = FragPos;
    normal = normalize(Normal);
    albedo = colour;
}