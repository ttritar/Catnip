#version 450
#extension GL_GOOGLE_include_directive : enable
#include "lighting_helpers.glsl"

// BUFFERS
layout(set = 0, binding = 0) uniform LightUBO {
    vec3 lightDir;     
    vec3 lightColor;  // luminance
    float lightIntensity; // lumen
    mat4 lightViewProj;

    vec4 cameraPos;  
    mat4 proj;
    mat4 view;
    vec2 viewportSize;

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
layout(set = 1, binding = 4) uniform sampler2D depthSampler;

layout(set = 2, binding = 0) uniform samplerCube environmentMap;
layout(set = 2, binding = 1) uniform samplerCube irradianceMap;

layout(set = 3, binding = 0) uniform sampler2D  shadowSampler; 


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
    float depthSample = texture(depthSampler, fragUV).r;


    
    // 0. Depth check for skybox
    if (depthSample >= 1.0)  // if theres nothing in front, render the skybox
    {
        vec2 fragCoord = vec2(gl_FragCoord.xy);
        vec3 viewDir = normalize(GetWorldPositionFromDepth(depthSample, fragCoord, ubo.viewportSize, inverse(ubo.proj), inverse(ubo.view)));
        outLit = vec4(texture(environmentMap, viewDir).rgb, 1.0);
        //outLit = vec4(normalize(viewDir), 1.0);
        return;
    }


    vec3 litColor = vec3(0.0);

    // 1. Directional Light
    vec3 directLight = CalculatePBR_Directional(albedoSample, normalSample, metallic, roughness, worldPosSample,
        ubo.lightDir, ubo.lightColor, ubo.lightIntensity, ubo.cameraPos.xyz);

    // 2. Point Lights
    uint pointLightCount = ubo.pointLightCount;
    for (int i = 0; i < pointLightCount; i++) 
    {
        PointLight pl = pointLights[i];

        vec3 L = pl.position.xyz - worldPosSample;
        float distance = length(L);

        if (distance < pl.radius) // radius check
        {
            float attenuation = 1.0 / (distance * distance + 0.00001);

            directLight += CalculatePBR_Point(albedoSample, normalSample, metallic, roughness, worldPosSample,
                pl.position.xyz, pl.color.rgb, pl.intensity * attenuation, ubo.cameraPos.xyz );
        }
    }


    // 3. Shadow
    float shdw = CalculateShadow( ubo.lightViewProj, ubo.lightDir, normalSample, worldPosSample, shadowSampler);
    litColor += directLight * shdw; 

    // 4. IBL
    vec3 iblColor = CalculateDiffuseIrradiance( irradianceMap, albedoSample, normalSample);
    litColor += iblColor;


    outLit = vec4(litColor, 1.0);
}

