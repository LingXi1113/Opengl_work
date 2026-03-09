#version 330 core

out vec4 FragColor;

in VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

uniform sampler2D texture_diffuse1; // treated as albedo
uniform vec3 viewPos;

// simple single-point light (matches how house lighting is configured)
uniform vec3 lightPos;
uniform vec3 lightColor;

uniform float metallic;  // 0..1
uniform float roughness; // 0.04..1
uniform float ao;        // 0..1

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / max(denom, 1e-6);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / max(denom, 1e-6);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

void main()
{
    // Treat diffuse map as sRGB albedo and convert to linear.
    vec3 albedo = pow(texture(texture_diffuse1, fs_in.TexCoords).rgb, vec3(2.2));

    float m = clamp(metallic, 0.0, 1.0);
    float r = clamp(roughness, 0.04, 1.0);
    float a = clamp(ao, 0.0, 1.0);

    vec3 N = normalize(fs_in.Normal);
    vec3 V = normalize(viewPos - fs_in.WorldPos);

    // calculate reflectance at normal incidence; if metallic, use albedo as F0
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, m);

    // lighting
    vec3 L = normalize(lightPos - fs_in.WorldPos);
    vec3 H = normalize(V + L);

    float distance    = length(lightPos - fs_in.WorldPos);
    float attenuation = 1.0 / max(distance * distance, 1e-4);
    vec3 radiance     = lightColor * attenuation;

    float NDF = DistributionGGX(N, H, r);
    float G   = GeometrySmith(N, V, L, r);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator    = NDF * G * F;
    float denom       = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular     = numerator / max(denom, 1e-6);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - m;

    float NdotL = max(dot(N, L), 0.0);
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

    // ambient (simple IBL-less approximation)
    vec3 ambient = vec3(0.08) * albedo * a;

    vec3 color = ambient + Lo;

    // Reinhard tonemap + gamma
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
