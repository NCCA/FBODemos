#version 330 core
layout(location=0)out vec4 fragColour;
layout(location=1)out vec4 brightness;
//in vec2 uv;

uniform sampler2D positionSampler;
uniform sampler2D normalSampler;
uniform sampler2D albedoMetallicSampler;
uniform sampler2D aoSampler;
uniform sampler2D ssaoSampler;
uniform bool useAO;
struct Light
{
    vec3 position;
    vec3 colour;
    float linear;
    float quadratic;
};


const float PI = 3.14159265359;
// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}
// ----------------------------------------------------------------------------



//const int NR_LIGHTS = @numLights;
uniform Light lights[@numLights];
uniform vec3 viewPos;
uniform vec2 screenResolution;

void main()
{

  vec2  uv=vec2(gl_FragCoord.x/screenResolution.x,gl_FragCoord.y/screenResolution.y);

  vec3 albedo     =   pow(texture(albedoMetallicSampler, uv).rgb, vec3(2.2));
  float metallic  = texture(albedoMetallicSampler,uv).a;
  float roughness = texture(normalSampler, uv).a;



  //float ao        = texture(aoSampler, uv).r;
  float ambientOcclusion = texture(ssaoSampler, uv).r;

  vec3 WorldPos = texture(positionSampler, uv).rgb;
  vec3 N = texture(normalSampler, uv).rgb;
  vec3 V = normalize(viewPos - WorldPos);

  // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
  // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metallic);
  // reflectance equation
  vec3 Lo = vec3(0.0);
  for(int i = 0; i < @numLights; ++i)
  {

    // calculate per-light radiance
    vec3 L = normalize(lights[i].position - WorldPos);
    vec3 H = normalize(V + L);
    float distance = length(lights[i].position - WorldPos);
    float attenuation = 1.0 / (1.0 + lights[i].linear * distance + lights[i].quadratic * distance * distance);

    vec3 radiance = lights[i].colour * attenuation;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, L, roughness);
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 nominator    = NDF * G * F;
    float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; // 0.001 to prevent divide by zero.
    vec3 specular = nominator / denominator;

    // kS is equal to Fresnel
    vec3 kS = F;
    // for energy conservation, the diffuse and specular light can't
    // be above 1.0 (unless the surface emits light); to preserve this
    // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
    // multiply kD by the inverse metalness such that only non-metals
    // have diffuse lighting, or a linear blend if partly metal (pure metals
    // have no diffuse light).
    kD *= 1.0 - metallic;

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);

    // add to outgoing radiance Lo
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;  // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again

  }

  vec3 ambient = vec3(0.1) * albedo *ambientOcclusion;


  if(useAO==true)
    fragColour = vec4( ambient+Lo, 1.0);
  else
    fragColour = vec4( Lo, 1.0);


  float bright = dot(fragColour.rgb, vec3(0.2126, 0.7152, 0.0722));
  if(bright > 1.0)
      brightness = vec4(fragColour.rgb, 1.0);
  else
      brightness = vec4(0.0, 0.0, 0.0, 1.0);

}

