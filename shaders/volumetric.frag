#version 450
#extension GL_GOOGLE_include_directive : enable
#include "volumetric_helpers.glsl"

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D sceneColor;
layout(set = 0, binding = 1) uniform sampler2D depthBuffer;
layout(set = 0, binding = 2) uniform sampler2D dirShadowMap;

layout(set = 0, binding = 3) uniform VolumetricsUBO
{
    mat4 invViewProj;
    mat4 lightViewProj;

    vec3 cameraPos;
    float _pad0;

    vec3 lightDir;
    float _pad1;

    vec3 lightColor;
    float lightIntensity;

    float fogDensity;
    float stepSize;
    int numSteps;

    float rayStrength;
    float rayDecay;
    float rayDensity;
    float rayWeight;
    
    int useMultipleScattering; // 0 = SS, 1 = MS
    float multiScatterStrength;
vec2 _pad2;

} ubo;


/* ================= God Rays ================= */
vec3 ComputeVolumetricGodRaysJittered(vec3 worldPos)
{
    vec3 viewDir = normalize(worldPos - ubo.cameraPos);
    float maxDistance = length(worldPos - ubo.cameraPos);

    vec3 rayAccum = vec3(0.0);
    float transmittance = 1.0;

    int steps = min(ubo.numSteps, 64);
    float stepSize = maxDistance / float(steps);

    float g = 0.95;
    float offset = fract(sin(dot(inTexCoord, vec2(12.9898,78.233))) * 43758.5453);
    float traveled = offset * stepSize;

    for (int i = 0; i < steps && traveled < maxDistance; ++i)
    {
        vec3 samplePos = ubo.cameraPos + viewDir * traveled;
        traveled += stepSize;

        float density = ubo.fogDensity * ubo.rayDensity;

        vec4 lightClip = ubo.lightViewProj * vec4(samplePos, 1.0);
        vec3 proj = lightClip.xyz / lightClip.w;

        if (abs(proj.x) <= 1.0 &&
            abs(proj.y) <= 1.0 &&
            proj.z >= 0.0 &&
            proj.z <= 1.0)
        {
            vec2 shadowUV = proj.xy * 0.5 + 0.5;
            float shadowDepth = texture(dirShadowMap, shadowUV).r;
            float visibility = (proj.z <= shadowDepth + 0.001) ? 1.0 : 0.0;

            float cosTheta = dot(normalize(-ubo.lightDir), viewDir);
            float phase = HenyeyGreensteinPhase(cosTheta, g);

            float directness = clamp(cosTheta * 2.0, 0.0, 1.0);
            directness = pow(directness, 3.0);

            vec3 contrib =
                ubo.lightColor *
                ubo.lightIntensity *
                density *
                visibility *
                phase *
                transmittance *
                (1.0 + directness * 2.0);

            rayAccum += contrib * stepSize;
        }

        transmittance *= exp(-density * stepSize);
        if (transmittance < 0.01)
            break;
    }

    return rayAccum * ubo.rayStrength;
}



/* ================= Scattering ================= */
struct FogResult
{
    vec3 light;
    float transmittance;
};

// SINGLE SCATTERING
FogResult SingleScattering(vec3 worldPos, vec3 viewDir, float maxDistance)
{
    vec3 marchPos = ubo.cameraPos;
    float traveled = 0.0;

    FogResult result;
    result.light = vec3(0.0);
    result.transmittance = 1.0;

    int steps = min(ubo.numSteps, int(maxDistance / ubo.stepSize));
    for (int i = 0; i < steps && traveled < maxDistance; ++i)
    {
        marchPos += viewDir * ubo.stepSize;
        traveled += ubo.stepSize;

        float density = ubo.fogDensity;

        vec4 lightClip = ubo.lightViewProj * vec4(marchPos, 1.0);
        vec3 proj = lightClip.xyz / lightClip.w;
        vec2 shadowUV = proj.xy * 0.5 + 0.5;
        shadowUV.y = 1.0 - shadowUV.y;

        float visibility = 0.0;
        if (abs(proj.x) <= 1.0 &&
            abs(proj.y) <= 1.0 &&
            proj.z >= 0.0 &&
            proj.z <= 1.0)
        {
            float shadowDepth = texture(dirShadowMap, shadowUV).r;
            visibility = (proj.z >= shadowDepth - 0.001) ? 1.0 : 0.0;
        }

        float cosTheta = dot(normalize(viewDir), normalize(ubo.lightDir));
        float phase = SchlickPhase(cosTheta, 0.82);

        vec3 scattering =
            ubo.lightColor *
            ubo.lightIntensity *
            density *
            visibility *
            phase;

        result.light += scattering * result.transmittance * ubo.stepSize;
        result.transmittance *= exp(-density * ubo.stepSize);

        if (result.transmittance < 0.01)
            break;
    }

    return result;
}


FogResult MultipleScattering(vec3 worldPos, vec3 viewDir, float maxDistance)
{
    vec3 marchPos = ubo.cameraPos;
    float traveled = 0.0;

    FogResult result;
    result.light = vec3(0.0);
    result.transmittance = 1.0;

    vec3 msLight = vec3(0.0); // accumulated indirect light

    int steps = min(ubo.numSteps, int(maxDistance / ubo.stepSize));
    for (int i = 0; i < steps && traveled < maxDistance; ++i)
    {
        marchPos += viewDir * ubo.stepSize;
        traveled += ubo.stepSize;

        float density = ubo.fogDensity;

        // === Shadowed direct lighting (same as SS) ===
        vec4 lightClip = ubo.lightViewProj * vec4(marchPos, 1.0);
        vec3 proj = lightClip.xyz / lightClip.w;
        vec2 shadowUV = proj.xy * 0.5 + 0.5;
        shadowUV.y = 1.0 - shadowUV.y;

        float visibility = 0.0;
        if (abs(proj.x) <= 1.0 &&
            abs(proj.y) <= 1.0 &&
            proj.z >= 0.0 &&
            proj.z <= 1.0)
        {
            float shadowDepth = texture(dirShadowMap, shadowUV).r;
            visibility = (proj.z >= shadowDepth + 0.001) ? 1.0 : 0.0;
        }

        float cosTheta = dot(normalize(viewDir), normalize(ubo.lightDir));
        float phase = SchlickPhase(cosTheta, 0.82);

        vec3 singleScatter =
            ubo.lightColor *
            ubo.lightIntensity *
            density *
            visibility *
            phase;

        // === MULTI-SCATTERING APPROXIMATION ===
        // Feed previously scattered light forward
        msLight += singleScatter * ubo.stepSize;

        // Attenuate MS light as it travels
        msLight *= exp(-density * ubo.stepSize);

        // === Accumulate both ===
        vec3 totalScattering = singleScatter + msLight;

        result.light += totalScattering * result.transmittance * ubo.stepSize;

        result.transmittance *= exp(-density * ubo.stepSize);

        if (result.transmittance < 0.01)
            break;
    }

    return result;
}





/* ================= Main ================= */
void main()
{
    float depth = texture(depthBuffer, inTexCoord).r;

    if (depth >= 0.99999)
    {
        outColor = vec4(texture(sceneColor, inTexCoord).rgb, 1.0);
        return;
    }

    vec3 worldPos = ReconstructWorldPos(
        inTexCoord,
        depth,
        ubo.invViewProj
    );

    vec3 viewDir = normalize(worldPos - ubo.cameraPos);
    float maxDistance = length(worldPos - ubo.cameraPos);

    FogResult fog =
        (ubo.useMultipleScattering == 1)
        ? MultipleScattering(worldPos, viewDir, maxDistance)
        : SingleScattering(worldPos, viewDir, maxDistance);

    vec3 godRays = ComputeVolumetricGodRaysJittered(worldPos);
    vec3 scene = texture(sceneColor, inTexCoord).rgb;

    vec3 finalColor = scene * fog.transmittance + fog.light + godRays;
    outColor = vec4(finalColor, 1.0);
}