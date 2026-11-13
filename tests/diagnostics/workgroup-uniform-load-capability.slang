// On GPU targets other than WGSL, `workgroupUniformLoad` executes a workgroup
// barrier in compute shaders, so it requires the `memorybarrier` capability. A
// caller declared with only a target must also require `memorybarrier`
// (shader-slang/slang#13631). The companion test
// workgroup-uniform-load-capability-ok.slang shows that it then compiles.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive): -target spirv -emit-spirv-directly -entry computeMain -stage compute

groupshared uint sharedVal;
RWStructuredBuffer<uint> outputBuffer;

[require(spirv)]
uint loadWithTargetOnly()
/*CHECK:
     ^^^^^^^^^^^^^^^^^^ 'loadWithTargetOnly' uses undeclared capability 'spirv_1_0'
*/
{
    return workgroupUniformLoad(sharedVal);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] = loadWithTargetOnly();
}
