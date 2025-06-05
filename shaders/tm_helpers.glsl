

vec3 LinearToSRGB(vec3 linear) 
{
    return mix(
        12.92 * linear,
        1.055 * pow(linear, vec3(1.0 / 2.4)) - 0.055,
        step(vec3(0.0031308), linear)
    );
}

vec3 Reinhard(vec3 color) 
{
    return color / (color + vec3(1.0));
}