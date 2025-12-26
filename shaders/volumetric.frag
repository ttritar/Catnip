#version 450

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D frameSampler;
layout(set = 0, binding = 1) uniform VolumetricsUBO
{
    vec2 ScreenLightPos;
    float Density;
    float Weight;
    float Decay;
    float Exposure;
    int NumSamples;
} ubo;


void main()
{
    // Calculate vector from pixel to light source in screen space
    vec2 deltaTexCoord = inTexCoord - ubo.ScreenLightPos;

    // Divide by number of samples and scale by density
    deltaTexCoord *= (1.0 / float(ubo.NumSamples)) * ubo.Density;

    // Initial sample
    vec3 color = texture(frameSampler, inTexCoord).rgb;
    float illuminationDecay = 1.0;
    vec2 texCoord = inTexCoord;

    // Ray marching
    for (int i = 0; i < ubo.NumSamples; ++i)
    {
        texCoord -= deltaTexCoord;
        vec3 sampleColor = texture(frameSampler, texCoord).rgb;
        sampleColor *= illuminationDecay * ubo.Weight;
        color += sampleColor;
        illuminationDecay *= ubo.Decay;
    }

    outColor = vec4(color * ubo.Exposure, 1.0);
}