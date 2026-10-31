//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

// A user-written constructor whose operands are a specialization-constant expression and
// literals has only module-scope operands. SPIR-V legalization does not hoist it, because a
// module-scope constructor is emitted as `OpConstantComposite`, which cannot take the
// `OpSpecConstantOp` result. It stays an `OpCompositeConstruct` in the function.
// See shader-slang/slang#13767.

// CHECK: %[[X:[0-9A-Za-z_]+]] = OpSpecConstant %uint 8
// CHECK: %[[XI:[0-9A-Za-z_]+]] = OpSpecConstantOp %int IAdd %[[X]]
// CHECK-NOT: OpConstantComposite %v3int %[[XI]]
// CHECK: OpCompositeConstruct %v3int %[[XI]] %int_2 %int_1

[vk::constant_id(0)]
const uint kX = 8;

//TEST_INPUT:ubuffer(data=[0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[noinline]
void writeVector(int3 v)
{
    outputBuffer[0] = v.x;
    outputBuffer[1] = v.y;
    outputBuffer[2] = v.z;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    // BUF: 8
    // BUF-NEXT: 2
    // BUF-NEXT: 1
    writeVector(int3(int(kX), 2, 1));
}
