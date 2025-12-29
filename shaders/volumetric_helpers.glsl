// volumetric_helpers.glsl
#ifndef VOLUMETRIC_HELPERS_GLSL
#define VOLUMETRIC_HELPERS_GLSL

const float PI = 3.14159265358979323846264338327950288;

vec3 ReconstructWorldPos(
    vec2 uv,
    float depth,
    mat4 invViewProj)
{
    vec4 clip = vec4(
        uv * 2.0 - 1.0,
        depth * 2.0 - 1.0,
        1.0
    );

    vec4 world = invViewProj * clip;
    return world.xyz / world.w;
}

float SchlickPhase(float cosTheta, float k)
{
    return (1.0 - k * k) /
           (4.0 * 3.14159265 * pow(1.0 + k * cosTheta, 2.0));
}

float HenyeyGreensteinPhase(float cosTheta, float g)
{
    float g2 = g * g;
    return (1.0 - g2) /
           (4.0 * 3.14159265 *
            pow(1.0 + g2 - 2.0 * g * cosTheta, 1.5));
}

#endif
