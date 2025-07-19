//TEST:LANG_SERVER:
// Test exact scenario from the original issue that should crash

// Simulate the multi-module scenario from the issue
interface ISampleGenerator
{
    // Empty interface for testing
}

struct BSDFContext
{
    float iorI;  ///< IOR from incidence medium
    float iorT;  ///< IOR trom transmission medium
    bool inited; ///< Flag to indicate if the struct was initialized

    __init(float iorI_, float iorT_)
    {
        iorI = iorI_;
        iorT = iorT_;
        inited = true;
    }

    __init()
    {
        iorI = 1.f;
        iorT = 1.f;
        inited = false;
    }
}

interface IBSDF
{
    float3 eval<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, BSDFContext bc);
}

// This struct has a PRIVATE __init (no public keyword) which causes the crash
struct LambertDiffuseBTDF : IBSDF
{
    float3 albedo = {}; ///< Diffuse albedo.
    
    // NOTE: This __init is PRIVATE (no public keyword) - this is the key to the crash
    __init(float3 albedo_)
    {
        this.albedo = albedo_;
    }

    float3 eval<S : ISampleGenerator>(const float3 wi, const float3 wo, inout S sg, BSDFContext bc)
    {
        if (min(wi.z, -wo.z) < 0.001)
            return float3(0.f);
        return float3(0.318309886f) * albedo * (-wo.z); // 1/PI approximation
    }
}

// This function call to private constructor should trigger the crash
LambertDiffuseBTDF createTranslucent(float3 color, float3 normal)
{
//HOVER:50,11
    return LambertDiffuseBTDF(color); // CRASH HERE - calling private __init
}

void main()
{
    let obj = createTranslucent(float3(1.0, 0.5, 0.2), float3(0, 0, 1));
}