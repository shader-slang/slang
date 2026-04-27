// When a user declares `__slang_coverage` with an incompatible type
// while `-trace-coverage` is active, the IR pass that locates the
// buffer must diagnose the mismatch and skip instrumentation. The
// diagnostic is anchored at the user's declaration so the failure is
// debuggable.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry computeMain -trace-coverage -profile cs_5_0

// `RWStructuredBuffer<float>` has the right outer shape but the wrong
// element type — both that and a totally-wrong outer type (`Buffer<uint>`,
// `Texture2D`, etc.) trigger the same diagnostic, since the user's fix
// is the same in both cases (declare it as `RWStructuredBuffer<uint>`).
RWStructuredBuffer<float> __slang_coverage;
//CHECK:                  ^^^^^^^^^^^^^^^^ warning E45100

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
    {
        __slang_coverage[0] = 1.0;
    }
}
