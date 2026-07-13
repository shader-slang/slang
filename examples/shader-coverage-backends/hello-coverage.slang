// hello-coverage.slang — the kernel from the user guide's shader
// coverage tutorial (docs/user-guide/a1-06-shader-coverage.md),
// dispatched for real by this example on a selectable backend.
//
// The control flow is arranged so a small input set produces partial
// coverage on purpose: with the default inputs (one negative value,
// gain fixed at 2.0), the `return value;` fallthrough in applyGain
// never runs, and the negative-clamp line runs exactly once.
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
