
// CONSTANTS
const float GAMMA = 2.2;


// TONE MAPPING
//------------------
vec3 Reinhard(vec3 color) 
{
    return color / (color + vec3(1.0));
}

vec3 Uncharted2ToneMappingCurve(in vec3 color)
{
    const float a = 0.15f;
    const float b = 0.50f;
    const float c = 0.10f;
    const float d = 0.20f;
    const float e = 0.02f;
    const float f = 0.30f;
    return ((color * (a * color + c * b) + d * e) / (color * (a * color + b) + d * f)) - e / f;
}

vec3 Uncharted2ToneMapping(in vec3 color)
{
    const float W = 11.2f;
    const vec3 curvedColor = Uncharted2ToneMappingCurve(color);
    float whiteScale = 1.f / Uncharted2ToneMappingCurve(vec3(W)).r;
    return clamp(curvedColor * whiteScale, 0.f, 1.f);
}



// PHYICAL CAM
//------------------
float CalculateEV100FromPhysicalCamera(in float aperture, in float shutterSpeed, in float ISO) 
{
    // EV100 = log2((aperture^2) / (shutterSpeed * ISO / 100.0))
    return log2((aperture * aperture) / (shutterSpeed * (ISO / 100.0)));
}

float ConvertEV100ToExposure(in float ev100) 
{
    const float maxLuminance = 1.2f * pow(2.f,ev100);
    return 1.0f / max(maxLuminance, 0.00001);
}

float CalculateEV100FromAverageLuminance(in float avgLum) 
{
    const float K = 12.5f;
    return log2((avgLum * 100.0f) / K);
}