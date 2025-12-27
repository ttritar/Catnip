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
    float _pad2;
} ubo;




vec3 ReconstructWorldPos(vec2 uv, float depth)
{
    // depth [0,1] to NDC [-1,1]
    vec4 clip = vec4(
        uv * 2.0 - 1.0,
        depth * 2.0 - 1.0,
        1.0
    );

    vec4 world = ubo.invViewProj * clip;
    return world.xyz / world.w;
}

float SchlickPhase(float cosTheta, float k)
{
    float numerator = 1.0 - k * k;
    float denominator = 4.0 * 3.14159265 * pow(1.0 - k * cosTheta, 2.0);
    return numerator / denominator;
}

void main()
{
    float depth = texture(depthBuffer, inTexCoord).r;
    
    // Sky pixels -> march far
    if (depth == 1.0)
    {
        depth = 0.9999;
    }
    
    vec3 worldPos = ReconstructWorldPos(inTexCoord, depth);
    vec3 viewDir = normalize(worldPos - ubo.cameraPos);
    
    float maxDistance = length(worldPos - ubo.cameraPos);
    
    // marching
    vec3 marchPos = ubo.cameraPos;
    float traveled = 0.0;
    
    vec3 fogLight = vec3(0.0);
    float transmittance = 1.0;
    
    for (int i = 0; i < ubo.numSteps && traveled < maxDistance; ++i)
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
        if (shadowUV.x >= 0.0 && shadowUV.x <= 1.0 &&
            shadowUV.y >= 0.0 && shadowUV.y <= 1.0 &&
            shadowUV.z >= 0.0 && shadowUV.z <= 1.0) 
        {
            // Manual depth comparison for sampler2D
            float shadowDepth = texture(dirShadowMap, shadowUV.xy).r;
            visibility = (shadowUV.z >= shadowDepth - 0.001) ? 1.0 : 0.0;
        }

        float cosTheta = dot(normalize(viewDir), normalize(ubo.lightDir));
        float phase = SchlickPhase(cosTheta, 0.82);
        vec3 scattering = ubo.lightColor * ubo.lightIntensity * density * visibility * phase;

    
        fogLight += scattering * transmittance * ubo.stepSize;
        transmittance *= exp(-density * ubo.stepSize);
    
        if (transmittance < 0.01)
            break;
    }

    vec3 scene = texture(sceneColor, inTexCoord).rgb;
    vec3 finalColor = scene * transmittance + fogLight;
    
    outColor = vec4(finalColor, 1.0);
}