//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

// Test that a uint specialization constant cast to int for use in spec constant
// arithmetic produces valid SPIRV. The cast requires a signedness reinterpretation
// via IAdd-by-zero since OpSpecConstantOp does not support OpBitcast and
// UConvert/SConvert require different bit widths.

// CHECK-DAG: %[[ZERO:[A-Za-z0-9_]+]] = OpConstant %int 0
// CHECK-DAG: %[[UINT_SC:[A-Za-z0-9_]+]] = OpSpecConstant %uint 10
// CHECK-DAG: OpSpecConstantOp %int IAdd %[[UINT_SC]] %[[ZERO]]
// CHECK-DAG: OpSpecConstantOp %int SDiv

[SpecializationConstant]
const uint A = 10;

[SpecializationConstant]
const int B = 2;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float arr[int(A) / B];
    for (int i = 0; i < int(A) / B; i++)
        arr[i] = float(i);
    float total = 0;
    for (int i = 0; i < int(A) / B; i++)
        total += arr[i];
    // int(A)/B = 5, sum = 0+1+2+3+4 = 10
    // BUF: 10
    outputBuffer[0] = total;
}
