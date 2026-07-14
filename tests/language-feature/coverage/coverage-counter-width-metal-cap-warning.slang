// Verifies that an explicitly requested 64-bit coverage counter width
// on a Metal target is capped with warning W45115 rather than silently
// rewritten: the user spelled out `-trace-coverage-counter-width 64`,
// so overriding that choice must be visible. The uncapped default (the
// user passed no width flag) stays silent; the emitted-MSL shape for
// both cases is pinned in `coverage-counter-width-metal-cap.slang`.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):-target metal -stage compute -entry computeMain -trace-coverage -trace-coverage-counter-width 64

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    uint value = tid.x;
    outputBuffer[0] = value;
}

// CHECK: 45115
// CHECK: capped to uint32
