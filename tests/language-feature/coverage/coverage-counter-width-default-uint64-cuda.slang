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

// The synthesized buffer is present in the emit. The emitter renames
// `__slang_coverage` (with two leading underscores in IR) to
// `_slang_coverage_0` (one leading underscore plus an index suffix)
// in generated source; the existing `coverage-cuda-atomics.slang`
// uses the same one-underscore prefix.
//CHECK-DAG: _slang_coverage

// CUDA `atomicAdd` overload selection follows the operand types: a
// 64-bit unsigned slot uses the `unsigned long long` overload. The
// emitted buffer's element type is `ulonglong` (CUDA's name for
// `uint64_t`) and the call-site value-operand is `1ULL`. Together
// these pin the default-uint64 path on CUDA.
//CHECK-DAG: RWStructuredBuffer<ulonglong>
//CHECK-DAG: atomicAdd
//CHECK-DAG: 1ULL
