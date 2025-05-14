//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type

// CHECK: %[[C0:[0-9A-Za-z_]+]] = OpSpecConstant %int 4
// CHECK: %[[I1:[0-9A-Za-z_]+]] = OpConstant %int 1
// CHECK: %[[COP0:[0-9A-Za-z_]+]] = OpSpecConstantOp %int BitwiseAnd %[[C0]] %[[I1]]
// CHECK: %[[I2:[0-9A-Za-z_]+]] = OpConstant %int 2
// CHECK: %[[COP1:[0-9A-Za-z_]+]] = OpSpecConstantOp %int BitwiseOr %[[COP0]] %[[I2]]
// CHECK: %[[COP2:[0-9A-Za-z_]+]] = OpSpecConstantOp %int IAdd %[[I1]] %[[COP1]]
// CHECK: %[[COP3:[0-9A-Za-z_]+]] = OpSpecConstantOp %int ShiftLeftLogical %[[C0]] %[[COP2]]
// CHECK: %[[ARR_TYPE:[0-9A-Za-z_]+]] = OpTypeArray %float %[[COP3]]
// CHECK: %[[PT_TYPE:[0-9A-Za-z_]+]] = OpTypePointer Function %[[ARR_TYPE]]

[SpecializationConstant]
const int constValue0 = 4;

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

static const int size = constValue0 << (1 + (constValue0 & 0x01 | 0x02));   // 4 << 3 = 32

// This test is to verify that if two array has the same spec constant size, they are the same type.
void func(out float buffer[constValue0 << (1 + (constValue0 & 0x01 | 0x02))], int idx)
{
    for (uint i = 0; i < size; i++)
    {
        buffer[i] = i;
    }
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint tid:SV_DispatchThreadID)
{
    float buffer[constValue0 << (1 + (constValue0 & 0x01 | 0x02))];
    // CHECK: OpVariable %[[PT_TYPE]] Function

    func(buffer, tid);

    float temp = buffer[0];
    for (uint i = 0; i < size; i++)
    {
        temp += buffer[i] * 2;
    }
    // Result will be (0 + size-1) * size = (0 + 31) * 32 * 2 = 992
    outputBuffer[0] = temp;
    // BUF: 992
}
