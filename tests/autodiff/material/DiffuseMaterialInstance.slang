//TEST_IGNORE_FILE:

__exported import IMaterialInstance;
__exported import IBSDF;

struct DiffuseBSDF : IBSDF
{
    float3 albedo;

    [TreatAsDifferentiable]
    float3 eval(const float3 wiLocal, const float3 woLocal)
    {
        return albedo;
    }
};

struct DiffuseMaterialInstance : IMaterialInstance
{
    DiffuseBSDF bsdf;

    [TreatAsDifferentiable]
    float3 eval(const float3 wi, const float3 wo)
    {
        return bsdf.eval(wi, wo);
    }
};
