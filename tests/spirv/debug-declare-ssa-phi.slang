// A local variable that SSA construction promotes loses its storage, so its
// debug variable is described by a DebugValue wherever a new SSA value for it
// comes into being: at each store, and at the merge point where the two
// incoming values meet in a phi.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -g2 -O0 -validate-ir

RWStructuredBuffer<int> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    int counter = 1;
    if (tid.x > 3)
        counter = 2;
    outputBuffer[0] = counter;
}

// CHECK: [[C_STR:%[0-9]+]] = OpString "counter"
// CHECK: [[C_DBG:%[A-Za-z0-9_]+]] = OpExtInst %void {{%[0-9]+}} DebugLocalVariable [[C_STR]]
// CHECK: DebugValue [[C_DBG]] %int_1
// CHECK: OpBranchConditional
// CHECK: DebugValue [[C_DBG]] %int_2
// CHECK: OpBranch [[MERGE:%[0-9]+]]
// CHECK: [[MERGE]] = OpLabel
// CHECK: [[MERGED:%[0-9]+]] = OpLoad %int
// CHECK: DebugValue [[C_DBG]] [[MERGED]]
