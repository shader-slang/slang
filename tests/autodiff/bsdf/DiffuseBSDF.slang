//TEST_IGNORE_FILE:

__exported import IBSDF;

struct DiffuseBSDF : IBSDF, IDifferentiable
{
    float3 albedo;

    [BackwardDifferentiable]
    float3 eval(const float3 wiLocal, const float3 woLocal)
    {
        return albedo * max(woLocal.z, 0.f) / M_PI;
    }
}

struct DiffuseMaterialInstance : IMaterialInstance
{
    ShadingData sd;
    DiffuseBSDF bsdf;

    [BackwardDifferentiable]
    float3 eval(const ShadingData sd, const float3 wo)
    {
        float3 wiLocal = no_diff sd.toLocal(sd.V);
        float3 woLocal = no_diff sd.toLocal(wo);
        return bsdf.eval(wiLocal, woLocal);
    }
}
