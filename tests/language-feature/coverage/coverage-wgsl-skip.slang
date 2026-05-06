// Verifies that `-trace-coverage` on a WGSL target produces an
// E45102 warning and skips coverage instrumentation. WGSL requires
// the buffer's element type to be `atomic<u32>` for `atomicAdd` and
// the IR coverage pass currently synthesizes plain `uint` — so
// instrumentation is skipped to avoid emitting invalid WGSL. The
// warning surfaces the limitation; the WGSL output remains valid
// (just uninstrumented).
//
// Regresses if `-target wgsl` starts succeeding silently with
// invalid WGSL, or if the warning code/severity changes.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl -entry main -trace-coverage -stage compute
//CHECK: warning E45102

// Verify the emitted WGSL is actually uninstrumented (the diagnostic
// alone doesn't tell us this — silent instrumentation that happens
// to match the warning would slip through).
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -trace-coverage -stage compute
//WGSL-NOT: _slang_coverage
//WGSL-NOT: atomicAdd

RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x == 0)
        outputBuffer[0] = 1;
}
