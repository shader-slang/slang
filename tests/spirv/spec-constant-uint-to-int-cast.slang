//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

// Test that integer specialization constants cast across signedness and/or width
// for use in spec constant arithmetic produce valid SPIRV.
//
// OpSpecConstantOp does not support OpBitcast and UConvert/SConvert require
// matching signedness, so signedness reinterpretation uses IAdd-by-zero.
//
// Case 1 (same-width, uint -> int): IAdd-by-zero
// Case 2 (cross-width, uint16_t -> int): UConvert then IAdd-by-zero
// Case 3 (same-width, int -> uint): IAdd-by-zero
// Case 4 (cross-width, int16_t -> uint): SConvert then IAdd-by-zero

// Case 1: uint -> int via IAdd-by-zero
// CHECK-DAG: %[[ZERO_INT:[A-Za-z0-9_]+]] = OpConstant %int 0
// CHECK-DAG: %[[A:[A-Za-z0-9_]+]] = OpSpecConstant %uint 10
// CHECK-DAG: OpSpecConstantOp %int IAdd %[[A]] %[[ZERO_INT]]
// CHECK-DAG: OpSpecConstantOp %int SDiv

// Case 2: uint16_t -> int via UConvert then IAdd-by-zero
// CHECK-DAG: %[[C:[A-Za-z0-9_]+]] = OpSpecConstant %ushort 3
// CHECK-DAG: %[[UCONV:[A-Za-z0-9_]+]] = OpSpecConstantOp %uint UConvert %[[C]]
// CHECK-DAG: OpSpecConstantOp %int IAdd %[[UCONV]] %[[ZERO_INT]]

// Case 3: int -> uint via IAdd-by-zero
// CHECK-DAG: %[[ZERO_UINT:[A-Za-z0-9_]+]] = OpConstant %uint 0
// CHECK-DAG: %[[D:[A-Za-z0-9_]+]] = OpSpecConstant %int 6
// CHECK-DAG: OpSpecConstantOp %uint IAdd %[[D]] %[[ZERO_UINT]]
// CHECK-DAG: OpSpecConstantOp %uint UDiv

// Case 4: int16_t -> uint via SConvert then IAdd-by-zero
// CHECK-DAG: %[[F:[A-Za-z0-9_]+]] = OpSpecConstant %short 4
// CHECK-DAG: %[[SCONV:[A-Za-z0-9_]+]] = OpSpecConstantOp %int SConvert %[[F]]
// CHECK-DAG: OpSpecConstantOp %uint IAdd %[[SCONV]] %[[ZERO_UINT]]

[SpecializationConstant]
const uint A = 10;

[SpecializationConstant]
const int B = 2;

[SpecializationConstant]
const uint16_t C = 3;

[SpecializationConstant]
const int D = 6;

[SpecializationConstant]
const uint E = 2;

[SpecializationConstant]
const int16_t F = 4;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    // Case 1: same-width uint -> int
    float arr1[int(A) / B];
    for (int i = 0; i < int(A) / B; i++)
        arr1[i] = float(i);
    float total1 = 0;
    for (int i = 0; i < int(A) / B; i++)
        total1 += arr1[i];
    // int(A)/B = 5, sum = 0+1+2+3+4 = 10
    // BUF: 10
    outputBuffer[0] = total1;

    // Case 2: cross-width uint16_t -> int
    float arr2[int(C)];
    for (int i = 0; i < int(C); i++)
        arr2[i] = float(i);
    float total2 = 0;
    for (int i = 0; i < int(C); i++)
        total2 += arr2[i];
    // int(C) = 3, sum = 0+1+2 = 3
    // BUF: 3
    outputBuffer[1] = total2;

    // Case 3: same-width int -> uint
    float arr3[int(uint(D) / E)];
    for (int i = 0; i < int(uint(D) / E); i++)
        arr3[i] = float(i);
    float total3 = 0;
    for (int i = 0; i < int(uint(D) / E); i++)
        total3 += arr3[i];
    // uint(D)/E = 3, sum = 0+1+2 = 3
    // BUF: 3
    outputBuffer[2] = total3;

    // Case 4: cross-width int16_t -> uint
    float arr4[int(uint(F) / E)];
    for (int i = 0; i < int(uint(F) / E); i++)
        arr4[i] = float(i);
    float total4 = 0;
    for (int i = 0; i < int(uint(F) / E); i++)
        total4 += arr4[i];
    // uint(F)/E = 2, sum = 0+1 = 1
    // BUF: 1
    outputBuffer[3] = total4;
}
