//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry first -stage compute -entry second -stage compute

// When the function that calls `WorkgroupSize()` is reachable from more than one entry point,
// the size is stored into a private global at the start of each entry point. Each stored
// vector converts the `uint` specialization constant to `int`, the same as the single-entry
// path, so the SPIR-V stays valid. See shader-slang/slang#13767.

// CHECK-DAG: %[[X:[0-9A-Za-z_]+]] = OpSpecConstant %uint 8
// CHECK-DAG: %[[XI:[0-9A-Za-z_]+]] = OpSpecConstantOp %int IAdd %[[X]]
// CHECK-DAG: %[[GV:[0-9A-Za-z_]+]] = OpVariable %_ptr_Private_v3int Private
// CHECK-NOT: OpConstantComposite %v3int %[[XI]]
// CHECK: %[[V1:[0-9A-Za-z_]+]] = OpCompositeConstruct %v3int %[[XI]] %int_1 %int_1
// CHECK-NEXT: OpStore %[[GV]] %[[V1]]
// CHECK: %[[V2:[0-9A-Za-z_]+]] = OpCompositeConstruct %v3int %[[XI]] %int_2 %int_1
// CHECK-NEXT: OpStore %[[GV]] %[[V2]]

[vk::constant_id(0)]
const uint kX = 8;

RWStructuredBuffer<int> outputBuffer;

[noinline]
void writeSize(int slot, int3 size)
{
    outputBuffer[slot] = size.x;
    outputBuffer[slot + 1] = size.y;
    outputBuffer[slot + 2] = size.z;
}

void reportSize(int slot)
{
    writeSize(slot, WorkgroupSize());
}

[shader("compute")]
[numthreads(kX, 1, 1)]
void first()
{
    reportSize(0);
}

[shader("compute")]
[numthreads(kX, 2, 1)]
void second()
{
    reportSize(3);
}
