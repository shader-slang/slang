//TEST(compute, vulkan):COMPARE_COMPUTE_EX:-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX:-slang -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

__exported import DiffuseBSDF;

[BackwardDifferentiable]
float L2Loss(float3 x, float3 xRef)
{
    float3 diff = x - xRef;
    return dot(diff, diff);
}

[BackwardDifferentiable]
float objectiveFunction(DiffuseBSDF curBSDF, no_diff DiffuseBSDF refBSDF)
{
    float3 wi = no_diff normalize(float3(0.3f, 0.2f, 0.8f));
    float3 wo = no_diff normalize(float3(-0.1f, -0.3f, 0.9f));

    float3 fRef = refBSDF.eval(wi, wo);
    float3 fCur = curBSDF.eval(wi, wo);
    return L2Loss(fCur, fRef);
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    DiffuseBSDF refBSDF = { float3(0.9f, 0.6f, 0.2f) };
    DiffuseBSDF curBSDF = { float3(0.5f, 0.5f, 0.5f) };

    DifferentialPair<DiffuseBSDF> dp_curBSDF = DifferentialPair<DiffuseBSDF>(
        { float3(0.5f, 0.5f, 0.5f) }, { float3(0.f) });

    __bwd_diff(objectiveFunction)(dp_curBSDF, refBSDF, 1.f);

    outputBuffer[0] = dp_curBSDF.d.albedo[0];
    outputBuffer[1] = dp_curBSDF.d.albedo[1];
    outputBuffer[2] = dp_curBSDF.d.albedo[2];
}
