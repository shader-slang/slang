//TEST_IGNORE_FILE:

struct MaterialInstanceData : IDifferentiable
{
    float data[5];
}

/*
struct MaterialInstanceSetupResult : IDifferentiable
{
    IMaterialInstance mi;
    MaterialInstanceData data;
}
*/

interface IMaterialInstance
{
    [BackwardDifferentiable]
    float3 eval(const MaterialInstanceData miData, const float3 wi, const float3 wo);
}
