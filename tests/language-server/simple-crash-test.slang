//TEST:LANG_SERVER:
// Simple test to reproduce the crash with private constructor

struct LambertDiffuseBTDF
{
    float3 albedo; ///< Diffuse albedo.
    
    // NOTE: This __init is PRIVATE (no public keyword) - this should cause issues
    __init(float3 albedo_)
    {
        this.albedo = albedo_;
    }
}

// This function call to private constructor might trigger issues
LambertDiffuseBTDF createTranslucent(float3 color)
{
//HOVER:16,11
    return LambertDiffuseBTDF(color); // Should trigger issues with private __init
}

void main()
{
    let obj = createTranslucent(float3(1.0, 0.5, 0.2));
}