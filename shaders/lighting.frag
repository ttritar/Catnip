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
const float SPECULAR_POWER = 4.0;
const float SHININESS_MUL = 100.0;
const float MIN_ROUGHNESS = 0.01; // NO ZERO DIV ONG




// FUNCTION DECLARATIONS
vec4 CalculateLighting(vec3 albedoSample, vec3 normalSample, vec3 specularSample, float roughness, vec3 worldPosSample,
    vec3 lightDir, vec3 lightColor, float lightIntensity, vec3 cameraPos);
float ObservedAreaShading(vec3 normal, vec3 lightDir);
vec3 DiffuseShading(vec3 albedo, vec3 normal, vec3 lightDir, vec3 lightColor, float lightIntensity);
vec3 SpecularShading(vec3 normal, vec3 worldPos, vec3 lightDir, vec3 lightColor, float lightIntensity, vec3 camPos, float specularStrength, float roughness);



void main()
{
    vec3 albedoSample = texture(albedoSampler, fragUV).rgb;
    vec3 normalSample = texture(normalSampler, fragUV).rgb;
    vec3 specularSample  = texture(specularSampler, fragUV).rgb;
    float specularStrength = specularSample.r;
    float roughness = max(specularSample.g, MIN_ROUGHNESS);
    vec3 worldPosSample = texture(worldSampler, fragUV).xyz;
    
    outLit = CalculateLighting(albedoSample, normalSample, specularSample, roughness, worldPosSample,
        ubo.lightDir, ubo.lightColor, ubo.lightIntensity, ubo.cameraPos);

    outLit.rgb = albedoSample;
    outLit.a = 1.0; // Set alpha to 1.0 for full opacity
}


// LIGHTING CALC
//----------------
vec4 CalculateLighting(vec3 albedoSample, vec3 normalSample, vec3 specularSample, float roughness, vec3 worldPosSample,
    vec3 lightDir, vec3 lightColor, float lightIntensity, vec3 cameraPos)
{
    vec3 diffuse = DiffuseShading(albedoSample, normalSample, lightDir, lightColor, lightIntensity);
    vec3 specularColor = SpecularShading(normalSample, worldPosSample, lightDir, lightColor, lightIntensity, cameraPos, specularSample.r, roughness);
    vec3 finalColor = diffuse + specularColor;

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

vec3 SpecularShading(vec3 normal, vec3 worldPos, vec3 lightDir, vec3 lightColor, float lightIntensity, vec3 camPos, float specularStrength, float roughness)
{
    vec3 vertexToCamera = normalize(camPos - worldPos);
    vec3 halfVector = normalize(vertexToCamera + normalize(lightDir));

    float dotNH = max(dot(normal, halfVector), 0.0);

    float shininess = pow(1.0 - roughness, SPECULAR_POWER) * SHININESS_MUL; 

    float specularFactor = pow(dotNH, shininess);
    
    vec3 specular = specularFactor * specularStrength * lightColor * lightIntensity;

    return specular;
}