//DISABLE_TEST:SIMPLE(filecheck=CHECK): -target spirv -capability abort -skip-spirv-validation

// Disabled because reverse-mode autodiff currently terminates abnormally when
// differentiating a function that calls abort(), matching the pre-existing
// printf() limitation. Enable this as a diagnostic or compilation test once
// reverse-mode reports a user-facing diagnostic or supports these calls.

// Document the current reverse-mode autodiff behavior for abort() in a
// differentiable function.

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
    var p = diffPair(2.0f, 0.0f);
    bwd_diff(f)(p, 1.0f);
    outBuf[0] = p.d;
}

// CHECK: OpCapability AbortKHR
// CHECK: OpAbortKHR
