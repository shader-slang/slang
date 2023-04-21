//TEST_IGNORE_FILE:

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

interface IBSDF
{
    [BackwardDifferentiable]
    float3 eval(const float3 wi, const float3 wo);
}
