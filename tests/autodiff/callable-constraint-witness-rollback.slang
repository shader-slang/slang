//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHK): -vk -shaderobj -output-using-type

// A [BackwardDifferentiable] interface method adds callable constraints to
// the requirement. A direct candidate with only a legacy backward derivative
// can satisfy the backward constraint but not the forward constraint, so it
// fails and conformance checking falls back to a synthesized wrapper.
//
// The failed direct candidate must not leave partial constraint witnesses in
// the witness table, or the synthesized wrapper will try to add the same
// backward constraint witness again.

interface IModule<T : IDifferentiable>
{
    [BackwardDifferentiable]
    T forward(T input);
}

struct InferenceModule : IModule<float>
{
    [BackwardDerivative(forward_bwd)]
    float forward(float input)
    {
        return input;
    }

    void forward_bwd(inout DifferentialPair<float> input, float dOut)
    {
        updateDiff(input, dOut);
    }
}

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    InferenceModule module;
    outputBuffer[dispatchThreadID.x] = module.forward(1.0);
    //CHK:1.0
}
