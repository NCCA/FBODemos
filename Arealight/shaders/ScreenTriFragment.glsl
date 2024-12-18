#version 410 core

uniform sampler2D position;
uniform sampler2D normal;
uniform sampler2D albedo;

in vec2 texCoord;
out vec4 fragColour;

uniform vec3 lightColour;
uniform int displayMode;

uniform vec3 lightPos;       // Center of the area light
uniform vec3 lightU;         // Direction vector for one side
uniform vec3 lightV;         // Direction vector for the other side
uniform vec3 lightIntensity; // Intensity of the area light
uniform vec3 viewPos;        // Camera position
uniform float specularPower;    // Shininess coefficient
uniform float specularStrength; // Strength of the specular term

const int NUM_SAMPLES = 16;  // Number of samples for soft shadows
const float EPSILON = 0.001; // Small offset to avoid self-shadowing

vec3 sampleLight(vec3 P, vec3 N, vec3 V, vec3 samplePoint, vec3 radiance) 
{
    vec3 L = samplePoint - P; // Vector from fragment to the light sample
    float dist = length(L);   // Distance to the light sample
    vec3 Ldir = normalize(L); // Direction to the light sample

    // Lambertian (diffuse) shading
    float NdotL = max(dot(N, Ldir), 0.0);

    // Specular shading (Phong model)
    vec3 R = reflect(-Ldir, N);           // Reflected light vector
    float RdotV = max(dot(R, V), 0.0);    // Specular angle
    float specular = pow(RdotV, specularPower) * specularStrength;

    // Attenuation based on the distance squared
    float attenuation = 1.0 / (dist * dist);

    // Combine diffuse and specular contributions
    return radiance * (NdotL + specular) * attenuation;
}





void main()
{
  // Lighting Pass
  // grab the components we need to do lighting
  vec3 position=texture(position,texCoord).xyz;
  vec3 N=texture(normal,texCoord).xyz;
  vec3 albedo=texture(albedo,texCoord).xyz;
  vec3 V = normalize(viewPos - position);
  vec3 radiance = vec3(0.0);
  vec3 L = normalize(lightPos - position);
 
 
 // Sample points on the light's surface
    for (int i = 0; i < NUM_SAMPLES; ++i) 
    {
     // Random offsets (simulate random sampling on the light's surface)
        float r1 = fract(sin(float(i) * 12.9898) * 43758.5453);
        float r2 = fract(sin(float(i) * 78.233) * 9631.5453);
        // Compute a sample point on the light's surface
        vec3 samplePoint = lightPos + r1 * lightU + r2 * lightV;
        // Add the contribution from this sample point
        radiance += sampleLight(position, N, V, samplePoint, lightIntensity / float(NUM_SAMPLES));
    }

    // Combine with surface color



  if (displayMode==0)
  {
  fragColour=vec4(pow(radiance * albedo,vec3(2.2)), 1.0);
  }
  else if (displayMode==1)
  {
    fragColour=vec4(position,1.0);
  }

  else if (displayMode==2)
  {
    fragColour=vec4(N,1.0);
  }
  else if (displayMode==3)
  {
    fragColour=vec4(vec3(1.0/2.2)*albedo,1.0);
  }
}