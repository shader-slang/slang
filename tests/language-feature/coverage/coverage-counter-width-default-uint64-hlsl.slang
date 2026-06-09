// Verifies that the default coverage counter width on HLSL targets is
// uint64. The synthesized buffer's element is `uint64_t` and the
// atomic-add uses DXC's `InterlockedAdd(uint64_t, ...)` overload,
// which requires Shader Model 6.6 and the `Int64BufferAtomics`
// feature. Callers targeting older HLSL profiles (SM 5.x / 6.0-6.5)
// must pass `-trace-coverage-counter-width 32`; that opt-down path is
// covered by the sibling SPIR-V test
// (`coverage-counter-width-opt-down-uint32-spirv.slang`).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry computeMain -profile cs_6_6 -trace-coverage

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = tid.x;
}

// HLSL: the buffer's element type is `uint64_t`, and the marker ops
// lower to `InterlockedAdd` calls. The element type plus the
// `InterlockedAdd` call together pin the default-width emission for
// HLSL.
//CHECK: __slang_coverage
//CHECK-DAG: uint64_t
//CHECK-DAG: InterlockedAdd
