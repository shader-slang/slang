// Test to reproduce the exact crash scenario from issue #7823
// This test reproduces the multi-module scenario with private __init that causes crashes

// First module - bsdf definitions
module bsdfs;

public struct BSDFContext
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

public interface IBSDF
{
    public float3 eval(const float3 wi, const float3 wo, BSDFContext bc);
}

// This struct has a PRIVATE __init (no public keyword) which causes the crash
public struct LambertDiffuseBTDF : IBSDF
{
    float3 albedo = {}; ///< Diffuse albedo.
    
    // NOTE: This __init is PRIVATE (no public keyword) - this is the key to the crash
    __init(float3 albedo_)
    {
        this.albedo = albedo_;
    }

    public float3 eval(const float3 wi, const float3 wo, BSDFContext bc)
    {
        return float3(0.318309886f) * albedo * (-wo.z); // 1/PI approximation
    }
}

// Second module - usage that triggers the crash
module make_bsdfs;
import bsdfs;

// This function call to private constructor should trigger the crash
LambertDiffuseBTDF createTranslucent(float3 color, float3 normal)
{
    return LambertDiffuseBTDF(color); // CRASH HERE - calling private __init
}

void main()
{
    let obj = createTranslucent(float3(1.0, 0.5, 0.2), float3(0, 0, 1));
}