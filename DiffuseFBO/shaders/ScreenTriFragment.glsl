#version 410 core

uniform sampler2D position;
uniform sampler2D normal;
uniform sampler2D albedo;

in vec2 texCoord;
out vec4 fragColour;

uniform vec3 lightPos;
uniform vec3 lightColour;

void main()
{
  // grab the components we need to do lighting
  vec3 position=texture(position,texCoord).xyz;
  vec3 normal=texture(normal,texCoord).xyz;
  vec3 albedo=texture(albedo,texCoord).xyz;
  // calculate the light direction 
  vec3 lightDir=normalize(lightPos-position);
  // calculate the diffuse term
  float diff=max(dot(normal,lightDir),0.0);
  // calculate the diffuse colour
  vec3 diffuse=diff*lightColour;
  fragColour=vec4(diffuse*albedo,1.0);
  
}