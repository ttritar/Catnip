#version 450
layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

layout(set = 1, binding = 0) uniform sampler2D litSampler;

void main() 
{
    vec4 texColor = texture(litSampler, fragUV);
    outColor = vec4(1, 0, 1, 1); // no discard, just always write


}