#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec3 inTangent;
layout(location = 5) in vec3 inBitangent;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outSpecular;
layout(location = 3) out vec4 outWorld;

layout(set = 1, binding = 0) uniform sampler2D albedoSampler;
layout(set = 1, binding = 1) uniform sampler2D normalSampler;
layout(set = 1, binding = 2) uniform sampler2D specularSampler;

void main() 
{
    // albedo
    outAlbedo = texture(albedoSampler, inUV);

    // normal
    vec3 normal = normalize(inNormal);
    vec3 tangent = normalize(inTangent);
    vec3 bitangent = normalize(inBitangent);
    
    vec3 biNormal = cross(normal, tangent);
    mat3 tangentSpace = mat3(tangent, biNormal, normal);
    vec3 sampledNormal = texture(normalSampler, inUV).rgb * 2.0 - 1.0;
    normal = normalize(sampledNormal * tangentSpace);

    outNormal = vec4(normal, 1.0);

    // specular
    vec3 spec = texture(specularSampler, inUV).rgb;
    outSpecular.b = spec.b;
    outSpecular.g = spec.g;

    // world position
    outWorld = vec4(inPosition, 1.0);
}