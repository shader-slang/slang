// Runtime smoke test for a module-scope varying input on Vulkan (shader-slang/slang#13687).
//
// `computeMain` reads a module-scope `SV_DispatchThreadID` and `WaveGetLaneCount()` through
// callees that a second entry point in the same file also calls. render-test compiles
// `computeMain` on its own, so this checks that the translated global still delivers the thread
// ID on the device; it does not exercise two entry points emitted into one module, which
// tests/wgsl/global-varying-multi-entry.slang and tests/spirv/global-varying-shared-in-*.slang
// cover. On SPIR-V, `WaveGetLaneCount()` reads the `SubgroupSize` builtin directly, so the lane
// count is only checked for a valid value here.

//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

in uint3 gThreadId : SV_DispatchThreadID;

uint threadIndex() { return gThreadId.x; }

// Every lane count Vulkan allows is a nonzero power of two.
uint laneCountIsValid()
{
    uint lanes = WaveGetLaneCount();
    return (lanes != 0 && (lanes & (lanes - 1)) == 0) ? 1 : 0;
}

[shader("compute")]
[numthreads(4, 1, 1)]
void otherMain()
{
    outputBuffer[threadIndex()] = 100 * laneCountIsValid();
}

// CHECK: 11
// CHECK-NEXT: 12
// CHECK-NEXT: 13
// CHECK-NEXT: 14
[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain()
{
    outputBuffer[threadIndex()] = threadIndex() + 10 * laneCountIsValid() + 1;
}
