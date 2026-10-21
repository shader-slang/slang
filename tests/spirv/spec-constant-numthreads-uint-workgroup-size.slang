//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

// `WorkgroupSize()` returns `int3`, so a `uint` specialization constant in `[numthreads]`
// is converted to `int` before it becomes a vector element. Without the conversion the
// generated SPIR-V builds a `v3int` from a `uint` constituent and fails validation.
// The converted value is an `OpSpecConstantOp`, so the vector is built with
// `OpCompositeConstruct` (an `OpConstantComposite` cannot take a spec constant).
// See shader-slang/slang#13767.

// CHECK-DAG: OpExecutionModeId %computeMain LocalSizeId %[[X:[0-9A-Za-z_]+]] %[[Y:[0-9A-Za-z_]+]] %{{[0-9A-Za-z_]+}}
// CHECK-DAG: %[[X]] = OpSpecConstant %uint 8
// CHECK-DAG: %[[Y]] = OpSpecConstant %int 4
// CHECK-DAG: %[[XI:[0-9A-Za-z_]+]] = OpSpecConstantOp %int IAdd %[[X]]
// CHECK-DAG: OpCompositeConstruct %v3int %[[XI]] %[[Y]]

[vk::constant_id(0)]
const uint kX = 8;

[vk::constant_id(1)]
const int kY = 4;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[noinline]
void writeSize(int3 size)
{
    outputBuffer[0] = size.x;
    outputBuffer[1] = size.y;
    outputBuffer[2] = size.z;
    outputBuffer[3] = size.x * size.y * size.z;
}

[numthreads(kX, kY, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (any(tid != uint3(0)))
        return;
    // BUF: 8
    // BUF-NEXT: 4
    // BUF-NEXT: 1
    // BUF-NEXT: 32
    writeSize(WorkgroupSize());
}
