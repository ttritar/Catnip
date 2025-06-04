#version 450

layout(set = 0, binding = 0) uniform LightUBO {
    vec3 lightDir;     
    vec3 lightColor;  // luminance
    float lightIntensity; // lumen

    vec3 cameraPos;  
} ubo;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outLit;

layout(set = 1, binding = 0) uniform sampler2D albedoSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;
layout(set = 1, binding = 2) uniform sampler2D specularSampler;
layout(set = 1, binding = 3) uniform sampler2D worldSampler;

const float PI = 3.14159265358979323846264338327950288;
const float MIN_ROUGHNESS = 0.045;
const vec3 DIELECTRIC_F0 = vec3(0.04);
const vec3 AMBIENT = vec3(0.03, 0.03, 0.03);



// FUNCTION DECLARATIONS
vec3 CalculatePBR(vec3 albedo, vec3 normal, float metallic, float roughness, vec3 worldPos,
    vec3 lightDir, vec3 lightColor, float lightIntensity, vec3 cameraPos);vec3 F_Schlick(vec3 F0, float cosTheta);
vec3 F_Schlick(vec3 F0, float cosTheta);
float D_GGX(float NdotH, float roughness);
float G_SchlickGGX(float NdotV, float roughness);
float G_Smith(float NdotV, float NdotL, float roughness);



void main()
{
    vec3 albedoSample = texture(albedoSampler, fragUV).rgb;
    vec3 normalSample = texture(normalSampler, fragUV).rgb;
    normalSample = normalize(normalSample * 2.0 - 1.0);
    vec3 specularSample  = texture(specularSampler, fragUV).rgb;

    // r = metalic, g = roughness
    float metallic = specularSample.r;
    float roughness = max(specularSample.g, MIN_ROUGHNESS);

    vec3 worldPosSample = texture(worldSampler, fragUV).xyz;
    

    vec3 litColor = CalculatePBR(albedoSample, normalSample, metallic, roughness, worldPosSample,
        ubo.lightDir, ubo.lightColor, ubo.lightIntensity, ubo.cameraPos);
    litColor = vec3(1.0) - exp(-litColor);

    outLit = vec4(litColor, 1.0);

}



// LIGHTING CALC
//----------------

vec3 F_Schlick(vec3 F0, float cosTheta)
{
    return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float D_GGX(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;

    return a2 / max(denom, 0.0001);
}

float G_SchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    return NdotV / (NdotV * (1.0 - k) + k);
}

float G_Smith(float NdotV, float NdotL, float roughness)
{
    return G_SchlickGGX(NdotV, roughness) * G_SchlickGGX(NdotL, roughness);
}

vec3 CalculatePBR(vec3 albedo, vec3 normal, float metallic, float roughness, vec3 worldPos,
    vec3 lightDir, vec3 lightColor, float lightIntensity, vec3 cameraPos)
{
    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPos - worldPos);
    vec3 L = normalize(-lightDir);
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0001);
    float NdotV = max(dot(N, V), 0.0001);
    float NdotH = max(dot(N, H), 0.0001);
    float LdotH = max(dot(L, H), 0.0001);

    // Radiance
    vec3 radiance = lightColor * lightIntensity;

    // Reflectance
    vec3 F0 = mix(DIELECTRIC_F0, albedo, metallic);

    // Fresnel
    vec3 F = F_Schlick(F0, LdotH);

    // Distribution
    float D = D_GGX(NdotH, roughness);

    // Geometry
    float G = G_Smith(NdotV, NdotL, roughness);


    // Cook-Torrance BRDF
    //--------------------
        // Specular
    vec3 numerator = D * F * G;
    float denominator = 4.0 * NdotV * NdotL;
    vec3 specular = numerator / max(denominator, 0.0001);

        // Diffuse
    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;

        // Final color
    vec3 Lo = (diffuse + specular) * radiance * NdotL;
    vec3 ambient = AMBIENT * albedo;

    return ambient + Lo;
}