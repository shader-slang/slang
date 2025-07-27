//TEST:SIMPLE(filecheck=CHECK): -target spirv
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF):-vk -output-using-type -emit-spirv-directly

// CHECK: %[[C0:[0-9A-Za-z_]+]] = OpSpecConstant %int 5
// CHECK: %[[I3:[0-9A-Za-z_]+]] = OpConstant %int 3
// CHECK: %[[COP0:[0-9A-Za-z_]+]] = OpSpecConstantOp %int SRem %[[C0]] %[[I3]]
// CHECK: %[[I2:[0-9A-Za-z_]+]] = OpConstant %int 2
// CHECK: %[[COP1:[0-9A-Za-z_]+]] = OpSpecConstantOp %int IMul %[[I2]] %[[C0]]
// CHECK: %[[COP2:[0-9A-Za-z_]+]] = OpSpecConstantOp %int IAdd %[[COP0]] %[[COP1]]
// CHECK: %[[ARR_TYPE:[0-9A-Za-z_]+]] = OpTypeArray %float %[[COP2]]
// CHECK: %[[PT_TYPE:[0-9A-Za-z_]+]] = OpTypePointer Workgroup %[[ARR_TYPE]]
// CHECK: OpVariable %[[PT_TYPE]] Workgroup


[SpecializationConstant]
const int constValue0 = 5;

groupshared float buffer[constValue0 * 2 + constValue0 % 3];

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(12, 1, 1)]
void computeMain(uint3 id : SV_GroupThreadID)
{
    buffer[id.x] = id.x;

    GroupMemoryBarrier();
    if (id.x == 0)
    {
        static const int size = constValue0 * 2 + constValue0 % 3;
        float temp = 0;
        for (uint i = 0; i < size; i++)
        {
            temp = temp + buffer[i];
        }
        outputBuffer[0] = temp;
    }
    // Result will be (0 + size-1) * size / 2
    // BUF: 66
}
