// Companion GPU check for shader-slang/slang#13687.
//
// `computeMain` and `otherMain` are two compute entry points that read a module-scope
// `SV_DispatchThreadID` and `WaveGetLaneCount()` through shared callees. The test runs
// `computeMain`, which is declared after `otherMain`, both with `otherMain` present in the module
// and without it, and checks that the global varying translation still forwards both values to
// `computeMain` on the device. render-test generates code per entry point; the single-module
// WGSL/Metal emission is covered by tests/wgsl/global-varying-multi-entry.slang.

//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type -xslang -DWITH_OTHER_ENTRY_POINT
//TEST(compute, vulkan):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -output-using-type

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

in uint3 gThreadId : SV_DispatchThreadID;

uint threadIndex() { return gThreadId.x; }

// Every lane count Vulkan allows is a power of two, so this returns 1 whenever the lane count
// was actually forwarded into the global; a global that was never written reads as 0.
uint laneCountIsValid()
{
    uint lanes = WaveGetLaneCount();
    return (lanes != 0 && (lanes & (lanes - 1)) == 0) ? 1 : 0;
}

#ifdef WITH_OTHER_ENTRY_POINT
[shader("compute")]
[numthreads(4, 1, 1)]
void otherMain()
{
    outputBuffer[threadIndex()] = laneCountIsValid();
}
#endif

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
