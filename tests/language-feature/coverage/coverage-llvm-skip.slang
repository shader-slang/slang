// Verifies that coverage tracing on LLVM-emitted CPU targets produces
// an E45102 warning and skips coverage instrumentation. LLVM-emitted
// CPU targets do not have a verified lowering path for the synthesized
// coverage resource + atomic sequence.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target host-callable -emit-cpu-via-llvm -entry main -stage compute -trace-function-coverage
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target host-callable -emit-cpu-via-llvm -entry main -stage compute -trace-branch-coverage
//CHECK: warning E45102

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    if (tid.x != 0)
        return;
}
