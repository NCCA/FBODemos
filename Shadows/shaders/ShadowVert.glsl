#version 330 core

/// modified from the OpenGL Shading Language Example "Orange Book"
/// Roost 2002

uniform mat4 MV;
uniform mat4 MVP;
uniform mat3 normalMatrix;
uniform mat4 textureMatrix;
uniform vec3 LightPosition;
uniform  vec4  inColour;


layout (location=0) in  vec3  inVert;
layout (location=1) in  vec3  inNormal;
layout (location=2) in vec2 inUV;
out vec4  ShadowCoord;
out vec4  Colour;
void main()
{
  vec4 ecPosition = MV * vec4(inVert,1.0);
	vec3 ecPosition3 = (vec3(ecPosition)) / ecPosition.w;
	vec3 VP = LightPosition - ecPosition3;
	VP = normalize(VP);
	vec3 normal = normalize(normalMatrix * inNormal);
	float diffuse = max(0.0, dot(normal, VP));
  vec4 texCoord = textureMatrix * vec4(inVert,1.0);
	ShadowCoord   = texCoord;
	Colour  = vec4(diffuse * inColour.rgb, inColour.a);
  gl_Position    = MVP * vec4(inVert,1.0);
}

