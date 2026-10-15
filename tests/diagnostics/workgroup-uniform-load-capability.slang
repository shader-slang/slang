// On GPU targets other than WGSL, `workgroupUniformLoad` executes a workgroup
// barrier, so it requires the `memorybarrier` capability. A caller declared with only a target
// must also require `memorybarrier` (shader-slang/slang#13631).

//TEST:SIMPLE(filecheck=CHECK): -target spirv -emit-spirv-directly -entry computeMain -stage compute

groupshared uint sharedVal;
RWStructuredBuffer<uint> outputBuffer;

[require(spirv)]
uint loadWithTargetOnly()
{
    return workgroupUniformLoad(sharedVal);
}

[require(spirv, memorybarrier)]
uint loadWithBarrierCapability()
{
    return workgroupUniformLoad(sharedVal);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = loadWithTargetOnly() + loadWithBarrierCapability();
}

// CHECK: error[E36104]
// CHECK: 'loadWithTargetOnly' uses undeclared capability
// CHECK-NOT: 'loadWithBarrierCapability' uses undeclared capability
