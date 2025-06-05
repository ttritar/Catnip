#version 450
#extension GL_GOOGLE_include_directive : enable
#include "tm_helpers.glsl"

layout(set = 0, binding = 0) uniform sampler2D litSampler;

layout(set = 0, binding = 1) uniform ToneMappingUniforms {
    float aperture;
    float shutterSpeed;
    float iso;
} tmUbo;

layout(location = 0) in vec2 fragUV;

layout(location = 0) out vec4 outColor;




void main() 
{
    vec3 litColor = texture(litSampler, fragUV).rgb;

    // TONE MAPPING
    //---------------

    // Calculate EV100
    float ev100 = CalculateEV100FromPhysicalCamera(tmUbo.aperture, tmUbo.shutterSpeed,tmUbo.iso);

    // Calculate exposure
    float exposure = ConvertEV100ToExposure(ev100);

    // Reinhard
    vec3 mapped = Uncharted2ToneMapping(litColor * exposure);

    // Gamma Correction
    mapped = pow(mapped, vec3(1.0 / GAMMA));


    outColor = vec4(mapped, 1.0);
}