#version 450
#extension GL_GOOGLE_include_directive : enable
#include "lighting_helpers.glsl"

// BUFFERS
layout(set = 0, binding = 0) uniform LightUBO {
    vec3 lightDir;     
    vec3 lightColor;  // luminance
    float lightIntensity; // lumen

    vec3 cameraPos;  

    uint pointLightCount; 
} ubo;

struct PointLight {
    vec4 position;
    vec4 color;
    float intensity;
    float radius;
};

layout(set = 0, binding = 1) readonly buffer Pointlights{
    PointLight pointLights[];
};

// IN AND OUT
layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outLit;

layout(set = 1, binding = 0) uniform sampler2D albedoSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;
layout(set = 1, binding = 2) uniform sampler2D specularSampler;
layout(set = 1, binding = 3) uniform sampler2D worldSampler;


// CONSTANTS
const float MIN_ROUGHNESS = 0.045;


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
    

    vec3 litColor = vec3(0.0);

    // 1. Directional Light
    litColor += CalculatePBR_Directional(albedoSample, normalSample, metallic, roughness, worldPosSample,
    ubo.lightDir, ubo.lightColor, ubo.lightIntensity, ubo.cameraPos);

    // 2. Point Lights
    uint pointLightCount = ubo.pointLightCount;
    for (int i = 0; i < pointLightCount; i++) 
    {
        PointLight pl = pointLights[i];

        vec3 L = pl.position.xyz - worldPosSample;
        float distance = length(L);

        if (distance < pl.radius) // radius check
        {
            float attenuation = 1.0 / (distance * distance + 0.0001);

            litColor += CalculatePBR_Point(albedoSample, normalSample, metallic, roughness, worldPosSample,
                pl.position.xyz, pl.color.rgb, pl.intensity * attenuation, ubo.cameraPos );
        }
    }

    outLit = vec4(litColor, 1.0);

}

