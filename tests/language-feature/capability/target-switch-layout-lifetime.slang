//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -DRESOURCE_ONLY
//TEST:SIMPLE(filecheck=CHECK): -target hlsl -entry computeMain -stage compute -DRESOURCE_ONLY
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -DRESOURCE_ONLY

// CUDA: result code = 0
// CUDA-NEXT: standard error = {
// CUDA-NEXT: }
// CUDA: __global__ void computeMain

// Linking must preserve the program layout through target-switch pruning, while later passes
// can still remove unused resource parameters on HLSL/SPIR-V without preserve-params.
// CUDA currently retains unused resources in its parameter struct.
// CHECK: result code = 0
// CHECK-NEXT: standard error = {
// CHECK-NEXT: }
// CHECK-NOT: unusedResource
// CHECK: computeMain
// CHECK-NOT: unusedResource

RWStructuredBuffer<uint> unusedResource;
RWStructuredBuffer<uint> output;
#ifndef RESOURCE_ONLY
uniform uint inputValue;
#endif

uint routed(uint value)
{
    __target_switch
    {
    case cuda: return value + 1;
    default: return value + 2;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
#ifdef RESOURCE_ONLY
    output[0] = routed(output[1]);
#else
    output[0] = routed(inputValue);
#endif
}
