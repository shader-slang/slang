//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability abort

// Verify that abort() is treated as a non-differentiable instruction in
// forward-mode autodiff, like printf(): differentiating a function that
// calls abort() must transcribe the call unchanged instead of crashing.

[Differentiable]
float f(float x)
{
    if (x > 1.0e6f)
        abort("input too large: %f", x);
    return x * x;
}

RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    var p = diffPair(2.0f, 1.0f);
    outBuf[0] = fwd_diff(f)(p).d;
}

// CHECK: OpCapability AbortKHR
// CHECK: OpAbortKHR
