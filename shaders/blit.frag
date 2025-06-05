#version 450
#extension GL_GOOGLE_include_directive : enable
#include "tm_helpers.glsl"

layout(set = 0, binding = 0) uniform sampler2D litSampler;

layout(set = 0, binding = 1) uniform ToneMappingUniforms {
    float exposure;         
    float gamma;            
} tmParams;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;

void main() 
{
    vec3 litColor = texture(litSampler, fragUV).rgb;

    // TONE MAPPING
    //---------------
    vec3 exposedColor = litColor * tmParams.exposure;
    vec3 toneMappedColor = Reinhard(exposedColor);
    vec3 finalColor = pow(toneMappedColor, vec3(1.0 / tmParams.gamma));

    outColor = vec4(finalColor, 1.0);
}