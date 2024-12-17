#version 410 core

uniform sampler2D position;
uniform sampler2D normal;
uniform sampler2D albedo;

in vec2 texCoord;
out vec4 fragColour;

uniform vec3 lightPos;
uniform vec3 lightColour;
uniform int displayMode;
void main()
{
  // Lighting Pass
  // grab the components we need to do lighting
  vec3 position=texture(position,texCoord).xyz;
  vec3 normal=texture(normal,texCoord).xyz;
  vec3 albedo=texture(albedo,texCoord).xyz;
  // calculate the light direction 
  vec3 lightDir=normalize(lightPos-position);
  float distance = length(lightPos - position);
  float attenuation = 1.0 / (distance * distance);
  vec3 radiance = lightColour * attenuation;

  // calculate the diffuse term
  float diff=max(dot(normal,lightDir),0.0);
  // calculate the diffuse colour
  vec3 diffuse=diff*radiance;
  if (displayMode==0)
  {
  fragColour=vec4(diffuse*albedo,1.0);
  }
  else if (displayMode==1)
  {
    fragColour=vec4(position,1.0);
  }

  else if (displayMode==2)
  {
    fragColour=vec4(normal,1.0);
  }
  else if (displayMode==3)
  {
    fragColour=vec4(vec3(1.0/2.2)*albedo,1.0);
  }
}