#version 450

layout(location = 0) in vec3 inFragPosition;
layout(location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform samplerCube envMap;

// consts
const float PI = 3.14159265358979323846;

void main()
{
	vec3 normal = normalize(inFragPosition);

    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, normal));
    up = normalize(cross(normal, right));
    vec3 tangent = right;
    vec3 bitangent = up;

    vec3 irradiance = vec3(0.0);

    float nrSamples = 0.f;
    const float sampleDelta = 0.025f;
    for(float phi = 0.0f; phi < 2*PI; phi += sampleDelta)
    {
        for(float theta = 0.0f; theta < PI/2; theta += sampleDelta)
        {
            vec3 tangentSample = vec3(
                sin(theta) * cos(phi),
                sin(theta) * sin(phi),
                cos(theta)
            );

            vec3 sampleVec =
            tangentSample.x * tangent + tangentSample.y * bitangent +
            tangentSample.z * normal;

            irradiance += texture(envMap, sampleVec).rgb * cos(theta) * sin(theta);
            ++nrSamples;
        }
    }

    irradiance = PI * irradiance * (1.f / float(nrSamples));
    outFragColor = vec4(irradiance, 1.f);
}