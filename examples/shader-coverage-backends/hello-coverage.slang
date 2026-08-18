// hello-coverage.slang — a variant of the user-guide coverage
// tutorial's kernel (docs/user-guide/a3-01-shader-coverage.md) with
// one deliberate change: gain is fixed at 2.0 instead of varying per
// thread, so `return value;` in applyGain NEVER runs (the tutorial's
// version runs it once). With one negative input, the clamp line runs
// exactly once. main.cpp asserts those two counts by this file's line
// numbers and README.md shows them, so any edit that adds or removes
// a line here (including in this header) must update both.
StructuredBuffer<float> inputBuffer;
RWStructuredBuffer<float> outputBuffer;

float applyGain(float value, float gain)
{
    if (gain > 1.0)
        return value * gain;
    return value;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 threadId: SV_DispatchThreadID)
{
    uint index = threadId.x;
    float value = inputBuffer[index];
    if (value < 0.0)
        value = 0.0;
    outputBuffer[index] = applyGain(value, 2.0);
}
