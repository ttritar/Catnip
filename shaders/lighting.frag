#version 450

layout(set = 0, binding = 0) uniform LightUBO {
    vec3 lightDir;     
    vec3 lightColor;  
    float lightIntensity;

    vec3 cameraPos;   
} ubo;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outLit;

layout(set = 1, binding = 0) uniform sampler2D albedoSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;
layout(set = 1, binding = 2) uniform sampler2D specularSampler;
layout(set = 1, binding = 3) uniform sampler2D worldSampler;

const float PI = 3.14159265358979323846264338327950288;
const float SPECULAR_POWER = 32.0;




// FUNCTION DECLARATIONS
vec4 CalculateLighting(vec3 albedoSample, vec3 normalSample, vec3 specularSample, vec3 worldPosSample,
    vec3 lightDir, vec3 lightColor, float lightIntensity, vec3 cameraPos);
float ObservedAreaShading(vec3 normal, vec3 lightDir);
vec3 DiffuseShading(vec3 albedo, vec3 normal, vec3 lightDir, vec3 lightColor, float lightIntensity);
vec3 SpecularShading(vec3 normal, vec3 worldPos, vec3 lightDir, vec3 lightColor, float lightIntensity, vec3 camPos);




void main()
{
    vec3 albedoSample = texture(albedoSampler, fragUV).rgb;
    vec3 normalSample = texture(normalSampler, fragUV).rgb;
    vec3 specularSample  = texture(specularSampler, fragUV).rgb;
    vec3 worldPosSample = texture(worldSampler, fragUV).xyz;
    
    outLit = CalculateLighting(albedoSample, normalSample, specularSample , worldPosSample,
    ubo.lightDir, ubo.lightColor, ubo.lightIntensity,
        ubo.cameraPos);
}


// LIGHTING CALC
//----------------
vec4 CalculateLighting(vec3 albedoSample, vec3 normalSample, vec3 specularSample, vec3 worldPosSample,
    vec3 lightDir, vec3 lightColor, float lightIntensity, vec3 cameraPos)
{
    vec3 diffuse = DiffuseShading(albedoSample, normalSample, lightDir, lightColor, lightIntensity);
    vec3 specularColor = SpecularShading(normalSample, worldPosSample, lightDir, lightColor, lightIntensity, cameraPos);
    vec3 finalColor = diffuse + specularColor * specularSample;

    return vec4(finalColor, 1.0);
}

float ObservedAreaShading(vec3 normal, vec3 lightDir)
{
    float lambertCos = max(dot(normal, normalize(lightDir)), 0.0);
    return lambertCos;
}

vec3 DiffuseShading(vec3 albedo, vec3 normal, vec3 lightDir, vec3 lightColor, float lightIntensity)
{
    float lambertsCos = ObservedAreaShading(normal, lightDir);
    vec3 lightContribution = (lambertsCos * lightIntensity * lightColor * albedo) / PI;
    return lightContribution;
}

vec3 SpecularShading(vec3 normal, vec3 worldPos, vec3 lightDir, vec3 lightColor, float lightIntensity, vec3 camPos)
{
    vec3 vertexToCamera = normalize(camPos - worldPos);
    vec3 lightReflect = reflect(-lightDir, normal);

    float specularFactor = max(dot(vertexToCamera, lightReflect), 0.0);
    specularFactor = pow(specularFactor, SPECULAR_POWER);

    return specularFactor * lightColor * lightIntensity;
}
