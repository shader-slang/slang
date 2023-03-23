//TEST_IGNORE_FILE:

__exported import IMaterial;
__exported import DiffuseMaterialInstance;

struct DiffuseMaterial : IMaterial
{
    float3 baseColor;

    typedef DiffuseMaterialInstance MaterialInstance;

    DiffuseMaterialInstance setupMaterialInstance()
    {
        DiffuseMaterialInstance mi = { {baseColor} };
        return mi;
    }
};
