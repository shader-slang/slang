//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK: OpAtomicIAdd

import "test_module";

RWStructuredBuffer<Atomic<int>> input0;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 workGroup : SV_GroupID)
{
    atomicFunc(&input0[0]);
}