//TEST_IGNORE_FILE:

__exported import IMaterialInstance;
__exported import IBSDF;

struct GlossyBSDF : IBSDF
{
    float3 albedo;

    [TreatAsDifferentiable]
    //[BackwardDifferentiable]
    float3 eval(const float3 wiLocal, const float3 woLocal)
    {
        return albedo * albedo;
    }
};

struct GlossyMaterialInstance : IMaterialInstance
{
    GlossyBSDF bsdf;

    [TreatAsDifferentiable]
    //[BackwardDifferentiable]
    float3 eval(const float3 wi, const float3 wo)
    {
        return bsdf.eval(wi, wo);
    }
};
