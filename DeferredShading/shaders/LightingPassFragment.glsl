#version 330 core
layout(location=0)out vec4 fragColour;
layout(location=1)out vec4 brightness;
//in vec2 uv;

uniform sampler2D positionSampler;
uniform sampler2D normalSampler;
uniform sampler2D albedoSpecSampler;

struct Light
{
    vec3 position;
    vec3 colour;
    float linear;
    float quadratic;
};

//const int NR_LIGHTS = @numLights;
uniform Light lights[@numLights];
uniform vec3 viewPos;
uniform vec2 screenResolution;
void main()
{
//    ivec2 uv = ivec2(gl_FragCoord.xy);
     vec2  uv=vec2(gl_FragCoord.x/screenResolution.x,gl_FragCoord.y/screenResolution.y);
    // retrieve data from gbuffer
    vec3 FragPos = texture(positionSampler, uv).rgb;
    vec3 Normal = texture(normalSampler, uv).rgb;
    vec3 Diffuse = texture(albedoSpecSampler, uv).rgb;
    float Specular = texture(albedoSpecSampler, uv).a;

    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
    vec3 viewDir  = normalize(viewPos - FragPos);
    for(int i = 0; i < @numLights; ++i)
    {
        // diffuse
        vec3 lightDir = normalize(lights[i].position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lights[i].colour;
        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 36.0);
        vec3 specular = lights[i].colour * spec * Specular;
        // attenuation
        float distance = length(lights[i].position - FragPos);
        float attenuation = 1.0 / (1.0 + lights[i].linear * distance + lights[i].quadratic * distance * distance);
        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;
    }

    const float gamma = 1.8;
    const float exposure=1.0;
    vec3 mapped = vec3(1.0) - exp(-lighting * exposure);
   // Gamma correction
     mapped = pow(mapped, vec3(1.0 / gamma));

    fragColour = vec4(mapped, 1.0);


    float bright = dot(fragColour.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(bright > 1.0)
        brightness = vec4(fragColour.rgb, 1.0);
    else
        brightness = vec4(0.0, 0.0, 0.0, 1.0);


}
