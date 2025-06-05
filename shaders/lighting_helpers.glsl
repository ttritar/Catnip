// CONSTANTS
const float PI = 3.14159265358979323846264338327950288;
const vec3 DIELECTRIC_F0 = vec3(0.04);


// LIGHTING CALC
//----------------
vec3 F_Schlick(vec3 F0, float cosTheta)
{
    return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float D_GGX(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;

    return a2 / max(denom, 0.0001);
}

float G_SchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    return NdotV / (NdotV * (1.0 - k) + k);
}

float G_Smith(float NdotV, float NdotL, float roughness)
{
    return G_SchlickGGX(NdotV, roughness) * G_SchlickGGX(NdotL, roughness);
}



// LIGHTS
//-----------------
vec3 CalculatePBR_Directional(vec3 albedo, vec3 normal, float metallic, float roughness, vec3 worldPos,
    vec3 lightDir, vec3 lightColor, float lightIntensity, vec3 cameraPos)
{
    vec3 N = normalize(normal);
    vec3 V = normalize(cameraPos - worldPos);
    vec3 L = normalize(-lightDir);
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0001);
    float NdotV = max(dot(N, V), 0.0001);
    float NdotH = max(dot(N, H), 0.0001);
    float LdotH = max(dot(L, H), 0.0001);

    // Radiance
    vec3 radiance = lightColor * lightIntensity;

    // Reflectance
    vec3 F0 = mix(DIELECTRIC_F0, albedo, metallic);

    // Fresnel
    vec3 F = F_Schlick(F0, LdotH);

    // Distribution
    float D = D_GGX(NdotH, roughness);

    // Geometry
    float G = G_Smith(NdotV, NdotL, roughness);


    // Cook-Torrance BRDF
    //--------------------
        // Specular
    vec3 numerator = D * F * G;
    float denominator = 4.0 * NdotV * NdotL;
    vec3 specular = numerator / max(denominator, 0.0001);

        // Diffuse
    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;

        // Final color
    vec3 Lo = (diffuse + specular) * radiance * NdotL;

    return Lo;
}


vec3 CalculatePBR_Point(vec3 albedo, vec3 normal, float metallic, float roughness, vec3 worldPos,
    vec3 lightPos, vec3 lightColor, float lightIntensity, vec3 cameraPos)
{
    vec3 N = normalize(normal);
    vec3 L = normalize(lightPos - worldPos);
    vec3 V = normalize(cameraPos - worldPos);
    vec3 H = normalize(V + L);

    float distance = length(lightPos - worldPos);
    float attenuation = 1.0 / (distance * distance); // Inverse square falloff

    float NdotL = max(dot(N, L), 0.0001);
    float NdotV = max(dot(N, V), 0.0001);
    float NdotH = max(dot(N, H), 0.0001);
    float LdotH = max(dot(L, H), 0.0001);

    // Radiance
    vec3 radiance = lightColor * lightIntensity * attenuation;

    // Reflectance at normal incidence
    vec3 F0 = mix(DIELECTRIC_F0, albedo, metallic);

    // Fresnel
    vec3 F = F_Schlick(F0, LdotH);

    // Normal Distribution
    float D = D_GGX(NdotH, roughness);

    // Geometry
    float G = G_Smith(NdotV, NdotL, roughness);
    

    // Cook-Torrance BRDF
    //--------------------
        // Specular 
    vec3 numerator = D * F * G;
    float denominator = 4.0 * NdotV * NdotL;
    vec3 specular = numerator / max(denominator, 0.0001);

        // Diffuse
    vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;

        // Final color
    vec3 Lo = (diffuse + specular) * radiance * NdotL;

    return Lo;
}



// IBL (Image Based Lighting)
//----------------
vec3 GetWorldPositionFromDepth(in float depth, in vec2 uv, in vec2 res, in mat4 invProj, in mat4 invView) 
{
    vec2 ndc = vec2(
        (uv.x / res.x) * 2.0 - 1.0,
        (uv.y / res.y) * 2.0 - 1.0
    );

    const vec4 clipPos = vec4(ndc, depth, 1.0);

    vec4 viewPos = invProj * clipPos;
    viewPos /= viewPos.w;

    vec4 worldPos = invView * viewPos;
    return worldPos.xyz;
}