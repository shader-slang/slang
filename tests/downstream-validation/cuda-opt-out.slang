// `slang-test` adds `-validate-with-downstream-compiler` to a `SIMPLE` test with `-target cuda` that
// expects the exit code 0. The option `no-downstream-validation` keeps `slang-test` from doing
// that. This test emits code that NVRTC rejects, so it passes only if `slang-test` honors the
// option. Without the option, `slangc` would exit with -1 on a machine that has NVRTC.

//TEST:SIMPLE(filecheck=CHECK,no-downstream-validation): -target cuda -entry computeMain -stage compute

__target_intrinsic(cuda, "@@this is not valid code@@($0)")
float invalidFunction(float x);

RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // CHECK: @@this is not valid code@@
    outputBuffer[tid.x] = invalidFunction(1.0);
}
