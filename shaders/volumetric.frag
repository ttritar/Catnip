#version 450

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
    float _pad2;
} ubo;




vec3 ReconstructWorldPos(vec2 uv, float depth)
{
    // depth [0,1] to NDC [-1,1]
    vec4 clip = vec4(
        uv.x * 2.0 - 1.0,
        uv.y * 2.0 - 1.0, 
        depth * 2.0 - 1.0,
        1.0
    );

    vec4 world = ubo.invViewProj * clip;
    return world.xyz / world.w;
}

float SchlickPhase(float cosTheta, float k)
{
    return (1.0 - k * k) / (4.0 * 3.14159265 * pow(1.0 + k * cosTheta, 2.0));
}

float HenyeyGreensteinPhase(float cosTheta, float g)
{
    float g2 = g * g;
    return (1.0 - g2) / (4.0 * 3.14159265 * pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
}


vec3 ComputeVolumetricGodRaysJittered()
{
    float depth = texture(depthBuffer, inTexCoord).r;
    if (depth >= 0.9999) return vec3(0.0);
    
    vec3 worldPos = ReconstructWorldPos(inTexCoord, depth);
    vec3 viewDir = normalize(worldPos - ubo.cameraPos);
    float maxDistance = length(worldPos - ubo.cameraPos);
       

    vec3 rayAccum = vec3(0.0);
    float transmittance = 1.0;
    
    int lightSteps = min(ubo.numSteps, 64);
    float lightStepSize = maxDistance / float(lightSteps);
    
    float g = 0.95; 
    
    float offset = fract(sin(dot(inTexCoord, vec2(12.9898, 78.233))) * 43758.5453);
    float traveled = offset * lightStepSize;
    
    for(int i = 0; i < lightSteps && traveled < maxDistance; i++)
    {
        vec3 samplePos = ubo.cameraPos + viewDir * traveled;
        traveled += lightStepSize;
        
        float density = ubo.fogDensity * ubo.rayDensity;
        
        // Shadow test with early out for performance
        vec4 lightClip = ubo.lightViewProj * vec4(samplePos, 1.0);
        lightClip.xyz /= lightClip.w;
        
        // Check if in light frustum
        if (abs(lightClip.x) <= 1.0 && abs(lightClip.y) <= 1.0 && lightClip.z >= 0.0 && lightClip.z <= 1.0)
        {
            vec2 shadowUV = lightClip.xy * 0.5 + 0.5;

            float shadowDepth = texture(dirShadowMap, shadowUV).r;
            float visibility = (lightClip.z <= shadowDepth + 0.001) ? 1.0 : 0.0;
            
            // Calculate phase
            float cosTheta = dot(normalize(-ubo.lightDir), viewDir);
            float phase = HenyeyGreensteinPhase(cosTheta, g);
            
            // Boost rays when looking directly at light
            float directness = clamp(cosTheta * 2.0, 0.0, 1.0);
            directness = pow(directness, 3.0);
            
            vec3 lightContrib = ubo.lightColor * ubo.lightIntensity * 
                               density * visibility * phase * transmittance * (1.0 + directness * 2.0);
            
            rayAccum += lightContrib * lightStepSize;
        }
        
        transmittance *= exp(-density * lightStepSize);
        if(transmittance < 0.01) break;
    }
    
    return rayAccum * ubo.rayStrength;
}

void main()
{
    float depth = texture(depthBuffer, inTexCoord).r;
    
    if (depth >= 0.99999)
    {
        outColor = vec4(texture(sceneColor, inTexCoord).rgb, 1.0);
        return;
    }
    
    vec3 worldPos = ReconstructWorldPos(inTexCoord, depth);
    vec3 viewDir = normalize(worldPos - ubo.cameraPos);
    float maxDistance = length(worldPos - ubo.cameraPos);
    
    // marching
    vec3 marchPos = ubo.cameraPos;
    float traveled = 0.0;
    
    vec3 fogLight = vec3(0.0);
    float transmittance = 1.0;
    
    int actualSteps = min(ubo.numSteps, int(maxDistance / ubo.stepSize));
    for (int i = 0; i < actualSteps && traveled < maxDistance; ++i)
    {
        marchPos += viewDir * ubo.stepSize;
        traveled += ubo.stepSize;
    
        float density = ubo.fogDensity;
    
        // Shadow test (dir only)
        vec4 lightClip = ubo.lightViewProj * vec4(marchPos, 1.0);
        vec3 shadowUV = lightClip.xyz / lightClip.w;
        shadowUV = shadowUV * 0.5 + 0.5;
        shadowUV.y = 1.0 - shadowUV.y; 

        
        float visibility = 0.0;
        if (abs(lightClip.x) <= 1.0 && abs(lightClip.y) <= 1.0 && lightClip.z >= 0.0 && lightClip.z <= 1.0)
        {
            float shadowDepth = texture(dirShadowMap, shadowUV.xy).r;
            visibility = (lightClip.z >= shadowDepth - 0.001) ? 1.0 : 0.0;
        }

        float cosTheta = dot(normalize(viewDir), normalize(ubo.lightDir));
        float phase = SchlickPhase(cosTheta, 0.82); 
        vec3 scattering = ubo.lightColor * ubo.lightIntensity * density * visibility * phase;

    
        fogLight += scattering * transmittance * ubo.stepSize;
        transmittance *= exp(-density * ubo.stepSize);
    
        if (transmittance < 0.01)
            break;
    }
    
    vec3 godRays = ComputeVolumetricGodRaysJittered();
    vec3 scene = texture(sceneColor, inTexCoord).rgb;
    vec3 finalColor = scene * transmittance + fogLight + godRays;
    
    outColor = vec4(finalColor, 1.0);
}