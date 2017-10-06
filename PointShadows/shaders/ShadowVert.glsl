#version 330 core

// based on demo code from here https://learnopengl.com/#!Advanced-Lighting/Shadows/Point-Shadows

/// @brief the vertex passed in
layout (location=0) in vec3 inPos;
/// @brief the normal passed in
layout (location=2) in vec3 inNormal;
/// @brief the in uv
layout (location=1) in vec2 inUV;



out vec2 TexCoords;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vsOut;

uniform mat4 model;
uniform mat4 MVP;
uniform mat3 normalMatrix;
void main()
{
    vsOut.FragPos = vec3(model * vec4(inPos, 1.0));
    vsOut.Normal = normalMatrix * inNormal;
    vsOut.TexCoords = inUV;
    gl_Position = MVP * vec4(inPos, 1.0);
}
