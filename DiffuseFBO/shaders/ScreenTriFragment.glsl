#version 410 core

uniform sampler2D position;
uniform sampler2D normal;
uniform sampler2D albedo;

in vec2 texCoord;
out vec4 fragColour;

uniform vec3 lightPos;
uniform vec3 lightColour;
uniform vec3 clearColor;

void main()
{

  vec3 position=texture(position,texCoord).xyz;
  vec3 normal=texture(normal,texCoord).xyz;
  vec3 albedo=texture(albedo,texCoord).xyz;
  vec3 lightDir=normalize(lightPos-position);
  float diff=max(dot(normal,lightDir),0.0);
  vec3 diffuse=diff*lightColour;
  // nead to add clear colour for backgrond
  fragColour=vec4(diffuse*albedo+clearColor,1.0);
//  fragColour=vec4(diffuse*albedo,1.0);
  

  
}