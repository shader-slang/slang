//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

// CHECK: %[[C0:[0-9A-Za-z_]+]] = OpSpecConstant %int 32
// CHECK: %[[I2:[0-9A-Za-z_]+]] = OpConstant %int 2
// CHECK: %[[COP0:[0-9A-Za-z_]+]] = OpSpecConstantOp %int SDiv %[[C0]] %[[I2]]
// CHECK: %[[COP1:[0-9A-Za-z_]+]] = OpSpecConstantOp %int ShiftRightArithmetic %[[C0]] %[[I2]]
// CHECK: %[[COP2:[0-9A-Za-z_]+]] = OpSpecConstantOp %int IAdd %[[COP0]] %[[COP1]]
// CHECK: %[[ARR_TYPE:[0-9A-Za-z_]+]] = OpTypeArray %float %[[COP2]]
// CHECK: %[[PT_TYPE:[0-9A-Za-z_]+]] = OpTypePointer Function %[[ARR_TYPE]]

[SpecializationConstant]
const int constValue0 = 32;

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

static float g_buffer[constValue0 / 2 + (constValue0 >> 2)];

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float buffer[constValue0 / 2 + (constValue0 >> 2)];
    // CHECK: OpVariable %[[PT_TYPE]] Function

    static const int size = constValue0 / 2 + (constValue0 >> 2);   // 16 + 8 = 24
    for (uint i = 0; i < size; i++)
    {
        buffer[i] = i;
        g_buffer[i] = i * 2;
    }

    float temp1 = 0.0f;
    float temp2 = 0.0f;
    for (uint i = 0; i < size; i++)
    {
        temp1 += buffer[i] * 2;
        temp2 += g_buffer[i];
    }

    // Result will be (0 + size-1) * size = (0 + 23) * 24 = 552
    outputBuffer[0] = temp1;
    // BUF: 552

    outputBuffer[1] = temp2;
    // BUF-NEXT: 552
}
