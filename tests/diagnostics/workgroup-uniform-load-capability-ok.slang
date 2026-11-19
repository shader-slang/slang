// A caller of `workgroupUniformLoad` that requires `memorybarrier` alongside its
// target compiles without a capability diagnostic (shader-slang/slang#13631).

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -emit-spirv-directly -entry computeMain -stage compute

groupshared uint sharedVal;
RWStructuredBuffer<uint> outputBuffer;

[require(spirv, memorybarrier)]
uint loadWithBarrierCapability()
{
    return workgroupUniformLoad(sharedVal);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = loadWithBarrierCapability();
}

// CHECK-NOT: error[E
// CHECK: OpEntryPoint GLCompute
// CHECK: OpControlBarrier
