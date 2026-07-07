//TEST:SIMPLE(filecheck=CHECK-CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-HLSL): -target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-SPIRV): -target spirv -entry computeMain -stage compute

import "gh-11829\rtx\utility\common.slang";

RWStructuredBuffer<int> output;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain()
{
    output[0] = testFunction();
}

//CHECK-CUDA: extern "C" __global__ void computeMain()
//CHECK-HLSL: void computeMain()
//CHECK-SPIRV: %computeMain = OpFunction
