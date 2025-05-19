//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[[SpecializationConstant]] const float NEAR_PLANE = 0.1f;
[[SpecializationConstant]] const float FAR_PLANE = 256.0f;
[[SpecializationConstant]] const int NEAR_PLANE_INT = 1;
[[SpecializationConstant]] const int FAR_PLANE_INT = 256;

// `OpSpecConstantOp` can only contain integer operations when targeting Vulkan SPIRV, not floating-point operations.
// This test checks that floating-point operations that strictly contain specialization constant variables are not declared with `OpSpecContantOp`,
// while integer operations that strictly contain specializaton constant operands are declared as `OpSpecConstantOp`.

// CHECK-DAG: OpSpecConstant %float 256
// CHECK-DAG: OpSpecConstant %float 0.1
// CHECK-NOT: OpSpecConstantOp %float
// CHECK-DAG: OpSpecConstant %int 256
// CHECK-DAG: OpSpecConstant %int 1
// CHECK-DAG: OpSpecConstantOp %int ISub
// CHECK-DAG: OpSpecConstantOp %int IAdd
// CHECK-DAG: OpSpecConstantOp %int IMul

float linearDepth(float depth)
{
    float z = depth;
    return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));
}

int linearDepthInt(int depth)
{
    int z = depth;
    return (2 * NEAR_PLANE_INT * FAR_PLANE_INT) / (FAR_PLANE_INT + NEAR_PLANE_INT - z * (FAR_PLANE_INT - NEAR_PLANE_INT));
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // BUF: 253.98
    outputBuffer[0] =  linearDepth(1.0) + float(linearDepthInt(2));
}
