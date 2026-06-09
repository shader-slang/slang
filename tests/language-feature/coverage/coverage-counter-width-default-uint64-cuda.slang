// Verifies that the default coverage counter width on the CUDA target
// is uint64. The synthesized buffer's element is `uint64_t` and the
// atomic-add uses CUDA's `atomicAdd(ulonglong*, ...)` overload. This
// is the regression guard for the CUDA backend's default-width
// emission path; the opt-down to uint32 is covered separately for
// SPIR-V and CPU (no current opt-down test on CUDA).

//TEST:SIMPLE(filecheck=CHECK):-target cuda -stage compute -entry computeMain -trace-coverage

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = tid.x;
}

// The synthesized buffer is present and its element width is 64-bit.
//CHECK: __slang_coverage

// CUDA `atomicAdd` overload selection follows the operand types: a
// 64-bit unsigned slot uses the `unsigned long long` overload. The
// presence of `atomicAdd` plus an unsigned-long-long literal (e.g.
// `1ULL`) on the coverage slot is the regression guard for the
// default-width path.
//CHECK-DAG: atomicAdd
//CHECK-DAG: 1ULL
