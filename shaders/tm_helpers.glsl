
// CONSTANTS
const float GAMMA = 2.2;


vec3 Reinhard(vec3 color) 
{
    return color / (color + vec3(1.0));
}


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