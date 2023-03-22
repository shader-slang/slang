//TEST_IGNORE_FILE:

interface IMaterialInstance
{
    //[BackwardDifferentiable]
    float3 eval(const float3 wi, const float3 wo);
}
