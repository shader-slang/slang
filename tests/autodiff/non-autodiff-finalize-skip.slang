// A module with no autodiff constructs must still compile and produce correct
// results when the autodiff finalization passes (finalizeAutoDiffPass /
// lowerDiffTypeInfoInsts) are skipped for derivative-free modules.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float> outputBuffer;

struct Accumulator
{
    float value;
    [mutating]
    void add(float x) { value += x; }
}

float scaledSum(float x)
{
    Accumulator acc = {0.0};
    for (int i = 1; i <= 3; ++i)
        acc.add(x * float(i));
    return acc.value; // x * (1 + 2 + 3) = 6x
}

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = scaledSum(2.0f);
    outputBuffer[1] = scaledSum(0.5f);
    // CHECK: 12.0
    // CHECK: 3.0
}
