#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject 
{
    mat4 view;
    mat4 proj;
} ubo;

layout(push_constant) uniform pushConstant 
{
    mat4 model;
} ps;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;


layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec2 outUV;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out vec3 outTangent;
layout(location = 5) out vec3 outBitangent;

void main() 
{
    gl_Position = ubo.proj * ubo.view * ps.model * vec4(inPosition, 1.0);
    
    outPosition = (ps.model * vec4(inPosition, 1.0)).rgb;
    outColor = inColor;
    outUV = inUV;
    outNormal = normalize(mat3(ps.model) * inNormal);
    outTangent = normalize(mat3(ps.model) * inTangent);
    outBitangent = normalize(mat3(ps.model) * inBitangent);
}