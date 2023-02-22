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
