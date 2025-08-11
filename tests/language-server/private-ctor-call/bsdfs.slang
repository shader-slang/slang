module bsdfs;
__include lambert_diffuse_btdf;

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
