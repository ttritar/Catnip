#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inNormal;

layout(set = 0, binding = 0) uniform CameraUBO {
    mat4 proj;
    mat4 view;
} ubo;

layout(push_constant) uniform Push {
    mat4 model;
} push;

layout(location = 0) out vec2 fragUV;

void main() 
{
    fragUV = inUV;
    gl_Position = ubo.proj * ubo.view * push.model * vec4(inPosition, 1.0);
}