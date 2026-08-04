// Material dispatch for the BVH demo. Four material kinds; the smoke
// scene uses only diffuse and emissive; the stress scene exercises
// all four. Branch coverage shows which arms fired.

public enum MaterialKind
{
    Diffuse = 0,
    Emissive = 1,
    Metallic = 2,
    Debug = 3,
}

float3 diffuseShade(float3 normal, float3 rayDir)
{
    // Lambertian with a fixed key-light direction.
    float3 light = normalize(float3(0.3, 1.0, 0.5));
    float ndotl = max(0.0, dot(normal, light));
    return float3(0.7, 0.7, 0.7) * ndotl;
}

float3 emissiveShade(float3 normal, float3 rayDir, float t)
{
    // Distance-attenuated emission. Bright at close range, fades with
    // 1/t falloff after t=1.
    float falloff = (t < 1.0) ? 1.0 : 1.0 / t;
    return float3(1.0, 0.9, 0.6) * falloff;
}

float3 metallicShade(float3 normal, float3 rayDir)
{
    // Schlick-Fresnel weighted reflection magnitude as a stand-in
    // for full reflection (the demo doesn't trace secondary rays).
    float ndotv = max(0.0, dot(normal, -rayDir));
    float fresnel = pow(1.0 - ndotv, 5.0);
    float3 base = float3(0.8, 0.6, 0.4);
    return lerp(base, float3(1.0), fresnel);
}

float3 debugShade(float3 normal)
{
    // RGB visualization of the surface normal — only used by the
    // stress scene's "debug" material.
    return normal * 0.5 + 0.5;
}

public float3 evaluateMaterial(int materialId, float3 rayDir, float3 normal, float t)
{
    MaterialKind kind = MaterialKind(materialId);
    switch (kind)
    {
    case MaterialKind::Diffuse:
        return diffuseShade(normal, rayDir);
    case MaterialKind::Emissive:
        return emissiveShade(normal, rayDir, t);
    case MaterialKind::Metallic:
        return metallicShade(normal, rayDir);
    case MaterialKind::Debug:
        return debugShade(normal);
    default:
        return float3(1.0, 0.0, 1.0); // magenta = unknown material
    }
}

public float3 skyGradient(float3 dir)
{
    float t = clamp(dir.y * 0.5 + 0.5, 0.0, 1.0);
    float3 horizon = float3(0.7, 0.8, 1.0);
    float3 zenith = float3(0.1, 0.2, 0.5);
    return lerp(horizon, zenith, t);
}
