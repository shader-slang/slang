// A module with no autodiff constructs must still compile and produce correct
// results when the autodiff finalization passes (finalizeAutoDiffPass /
// lowerDiffTypeInfoInsts) are skipped for derivative-free modules.
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type

// A derivative-free module takes the skip path in linkAndOptimizeIR: the cheap
// stripAutoDiffDecorations runs (so eliminateDeadCode can drop the core-module
// autodiff builtins), but the whole-module finalizeAutoDiffPass is skipped.
//TEST:SIMPLE(filecheck=PASS): -target hlsl -entry computeMain -stage compute -dump-ir-after stripAutoDiffDecorations -dump-ir-after finalizeAutoDiffPass
// PASS-NOT: ### AFTER finalizeAutoDiffPass:
// PASS: ### AFTER stripAutoDiffDecorations:
// PASS-NOT: ### AFTER finalizeAutoDiffPass:

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
