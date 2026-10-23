//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

// When the only non-literal `[numthreads]` axis is a `uint` specialization constant, every
// `WorkgroupSize()` vector element is module-scope, so SPIR-V legalization would hoist the
// vector to an `OpConstantComposite`. The `int` conversion of the spec constant is an
// `OpSpecConstantOp`, which an `OpConstantComposite` cannot take, so the vector must stay an
// `OpCompositeConstruct` inside the function. See shader-slang/slang#13767.

// CHECK: %[[X:[0-9A-Za-z_]+]] = OpSpecConstant %uint 8
// CHECK: %[[XI:[0-9A-Za-z_]+]] = OpSpecConstantOp %int IAdd %[[X]]
// CHECK-NOT: OpConstantComposite %v3int %[[XI]]
// CHECK: OpCompositeConstruct %v3int %[[XI]] %int_2 %int_1

[vk::constant_id(0)]
const uint kX = 8;

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

[numthreads(kX, 2, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    if (any(tid != uint3(0)))
        return;
    // BUF: 8
    // BUF-NEXT: 2
    // BUF-NEXT: 1
    // BUF-NEXT: 16
    writeSize(WorkgroupSize());
}
