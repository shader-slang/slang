//TEST_IGNORE_FILE:

__exported import IMaterial;

IMaterial createMaterialClassConformance(int type, float3 value)
{
    return createDynamicObject<IMaterial, float3>(type, value);
}
