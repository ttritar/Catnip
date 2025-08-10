#version 450

layout(set = 0,binding = 0) uniform LightMatrixUBO  
{
    mat4 view;
    mat4 proj;
} ubo;


layout(push_constant) uniform PushConstant {
    mat4 model;
} pc;

layout(location = 0) in vec3 inPosition;

void main() {
    gl_Position = ubo.proj * ubo.view * pc.model * vec4(inPosition, 1.0);
}