//TEST:SIMPLE(filecheck=CHECK): -target spirv

// Test that a uint specialization constant cast to int for use in spec constant
// arithmetic produces valid SPIRV. The cast requires a signedness reinterpretation
// via ShiftLeftLogical-by-zero since OpSpecConstantOp does not support OpBitcast
// and UConvert/SConvert require different bit widths.

// CHECK-DAG: %[[UINT_SC:[A-Za-z0-9_]+]] = OpSpecConstant %uint 10
// CHECK-DAG: OpSpecConstantOp %int ShiftLeftLogical %[[UINT_SC]]
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
    outputBuffer[0] = arr[0];
}
