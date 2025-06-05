#version 450

layout(location = 0) in vec3 inFragPosition;
layout(location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform sampler2D equirectangularSampler;

const vec2 INV_ATAN = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v) 
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= INV_ATAN;
    uv += 0.5;
    return uv;
}

void main() 
{
    vec3 dir = normalize(inFragPosition);
    vec2 uv = SampleSphericalMap(dir);
    vec3 color = texture(equirectangularSampler, uv).rgb;
    outFragColor = vec4(color, 1.0);
}