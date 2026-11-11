//TEST:SIMPLE(filecheck=SPIRV): -target spirv-asm -entry computeMain
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry computeMain
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -vk -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK): -d3d12 -output-using-type

// A buffer subscript has get/set and ref accessors. Atomic.load must bind its receiver
// to the element's storage instead of copying the getter's value into a local atomic.
//TEST_INPUT: set counters = ubuffer(data=[37], stride=4);
RWStructuredBuffer<Atomic<uint>> counters;

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4);
RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // CHECK: 37
    outputBuffer[0] = counters[0].load();
}

// SPIRV: OpAtomicLoad
// HLSL: RWStructuredBuffer<uint
// HLSL: void computeMain
