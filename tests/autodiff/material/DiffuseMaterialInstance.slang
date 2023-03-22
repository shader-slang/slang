//TEST_IGNORE_FILE:

__exported import IMaterialInstance;
__exported import IBSDF;

struct DiffuseBSDF : IBSDF
{
    float3 albedo;

    float3 eval(const float3 wiLocal, const float3 woLocal)
    {
        return albedo;
        //return albedo * max(woLocal.z, 0.f) / 3.1415926;
    }
};

struct DiffuseMaterialInstance : IMaterialInstance
{
    DiffuseBSDF bsdf;

    float3 eval(const float3 wi, const float3 wo)
    {
        return bsdf.eval(wi, wo);
    }
};
