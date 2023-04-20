//TEST_IGNORE_FILE:

__exported import IMaterial;
__exported import DiffuseMaterialInstance;

struct DiffuseMaterial : IMaterial
{
    float3 baseColor;

    typedef DiffuseMaterialInstance MaterialInstance;

    [TreatAsDifferentiable]
    float3 getAlbedo(float3 albedo)
    {
        return albedo;
    }

    [BackwardDifferentiable]
    DiffuseMaterialInstance setupMaterialInstance(out MaterialInstanceData miData)
    {
        float3 albedo = getAlbedo(baseColor);
        DiffuseMaterialInstance mi = { baseColor };
        for (uint i = 0; i < 3; i++) miData.data[i] = albedo[i];
        return mi;
    }
};
