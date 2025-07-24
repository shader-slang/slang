implementing bsdfs;
public struct LambertDiffuseBTDF : IBSDF, IDifferentiable
{
    float3 albedo = {}; ///< Diffuse albedo.
#if 0
    public __init(float3 albedo_)
#else
    __init(float3 albedo_)
#endif
    {
        this.albedo = albedo_;
    }

    [Differentiable]
    public float3 eval(const float3 wi, const float3 wo, BSDFContext bc)
    {
        if (min(wi.z, -wo.z) < 1e-6f)
            return float3(0.f);

        return (1.0 / 3.1415) * albedo * -wo.z;
    }
}
