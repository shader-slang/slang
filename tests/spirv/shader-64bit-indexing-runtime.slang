// Runtime (GPU-in-the-loop) functional test for 64-bit indexing (issue #11967, follow-up to
// #11538 / PR #11541, and to the static codegen guard #12081 `shader-64bit-indexing-functional.slang`).
//
// The merged static test proves at the SPIR-V level that a >= 2^32 index reaches OpAccessChain
// un-truncated. This test complements it by putting a real Vulkan driver in the loop: it runs a
// compute shader that indexes a buffer through a `uint64_t` index (so the emitted access uses
// 64-bit index operands, gated by `[Shader64BitIndexing]` / `spvShader64BitIndexingEXT`) and
// verifies the values the driver actually writes back. This catches breakage in the 64-bit-indexing
// code path or in the driver's handling of 64-bit index operands that a codegen-only check cannot.
//
// This is a Vulkan-only test: `spvShader64BitIndexingEXT` is a SPIR-V extension, so the feature does
// not exist on non-SPIR-V targets (CUDA/HLSL/Metal reject `[Shader64BitIndexing]` with E36107). The
// index values stay within the small test buffer's range: a true >2^32-element access would need a
// >16 GB device buffer, which is impractical for the regression suite, so that boundary stays
// covered by the static guard. What this test adds is confirming a real driver accepts and executes
// a 64-bit-typed index operand end-to-end.

//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -capability spvShader64BitIndexingEXT

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[Shader64BitIndexing]
[numthreads(8, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint64_t index = (uint64_t)dispatchThreadID.x;
    outputBuffer[index] = int(dispatchThreadID.x) + 1;
}

// CHECK: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8
