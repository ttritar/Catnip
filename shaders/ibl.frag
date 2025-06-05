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
    up = normalize(cross(normal, normalize(cross(up, normal))));
	vec3 tangent = normalize(cross(up, normal));
    vec3 bitangent = cross(normal, tangent);


	// MONTE CARLO
    const uint SAMPLE_COUNT = 512;
    vec3 irradiance = vec3(0.0);
    
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        float xi1 = float(i) / float(SAMPLE_COUNT);
        float xi2 = fract(float(i) * 0.61803398875); 
        
        float phi = 2 * PI * xi1;
        float sinTheta = sqrt(xi2);
        float cosTheta = sqrt(1.0 - xi2);
        
        vec3 hemisphereSample = vec3(
            sinTheta * cos(phi),
            sinTheta * sin(phi),
            cosTheta
        );
        
        vec3 sampleVec = hemisphereSample.x * tangent +
                        hemisphereSample.y * bitangent +
                        hemisphereSample.z * normal;
        
        irradiance += texture(envMap, sampleVec).rgb * cosTheta;
    }
    
    irradiance = PI * irradiance * (1.0 / float(SAMPLE_COUNT));
    outFragColor = vec4(irradiance, 1.0);
}