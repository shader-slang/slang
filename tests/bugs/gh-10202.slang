//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -output-using-type
// CHECK: 0
//TEST_INPUT:ubuffer(data=[0], stride=8):out,name=outputBuffer

// Regression test: constant folding >> 64 on uint64_t produces wrong value (UB in compiler)
// Issue: https://github.com/shader-slang/slang/issues/10202
// Expected: outputBuffer[0] == 0. Actual: outputBuffer[0] == 0x8000000000000000 (unchanged).

RWStructuredBuffer<uint64_t> outputBuffer;

[numthreads(1,1,1)]
void computeMain() {
    const uint64_t topBitSet = 0x8000000000000000ull;
    const uint64_t shifted = topBitSet >> 64;
    outputBuffer[0] = shifted;
}
