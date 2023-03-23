//TEST_IGNORE_FILE:

interface IBSDF
{
    [BackwardDifferentiable]
    float3 eval(const float3 wi, const float3 wo);
}
