//TEST_IGNORE_FILE:

__exported import IMaterial;
__exported import GlossyMaterialInstance;

struct GlossyMaterial : IMaterial, IDifferentiable
{
    float3 baseColor;

    typedef GlossyMaterialInstance MaterialInstance;

    GlossyMaterialInstance setupMaterialInstance()
    {
        GlossyMaterialInstance mi = { { baseColor } };
        return mi;
    }
};
