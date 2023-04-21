//TEST_IGNORE_FILE:

__exported import IMaterialInstance;
__exported import IBSDF;

struct GlossyMaterialInstance : IMaterialInstance
{
    float3 albedo;

    [BackwardDifferentiable]
    float3 eval(const MaterialInstanceData miData, const float3 wi, const float3 wo)
    {
        float3 albedo = float3(0.0f, 0.0f, 0.0f);
        for (uint i = 0; i < 3; i++) albedo[i] = miData.data[i];
        return albedo * albedo;
    }
};
