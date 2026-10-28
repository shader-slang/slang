// A built-in call on a specialization constant stays symbolic in the front end and lowers to
// SPIR-V `OpSpecConstantOp` compare and select instructions, so it can size a groupshared array
// (shader-slang/slang#13973).
//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute
//TEST(compute, vulkan):COMPARE_COMPUTE(filecheck-buffer=BUF): -vk -output-using-type

// CHECK: %[[FOO:[0-9A-Za-z_]+]] = OpSpecConstant %int 4
// CHECK: %[[CMP:[0-9A-Za-z_]+]] = OpSpecConstantOp %bool SGreaterThan %[[FOO]] %{{[0-9A-Za-z_]+}}
// CHECK: %[[SIZE:[0-9A-Za-z_]+]] = OpSpecConstantOp %int Select %[[CMP]] %[[FOO]] %{{[0-9A-Za-z_]+}}
// CHECK: %[[ARR_TYPE:[0-9A-Za-z_]+]] = OpTypeArray %float %[[SIZE]]
// CHECK: OpTypePointer Workgroup %[[ARR_TYPE]]

[vk::specialization_constant]
const int FOO = 4;

groupshared float arr[max(FOO, 1)];

//TEST_INPUT:ubuffer(data=[0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint tid : SV_GroupIndex)
{
    arr[tid] = float(tid * 2);
    GroupMemoryBarrierWithGroupSync();
    if (tid == 0)
    {
        // With FOO == 4 each call picks a different operand than the plain value of FOO.
        int arrMin[min(FOO, 3)];
        int arrClamp[clamp(FOO, 1, 2)];
        int arrAbs[abs(FOO - 9)];
        outputBuffer[0] = int(arr[3]);
        outputBuffer[1] = arr.getCount();
        outputBuffer[2] = arrMin.getCount();
        outputBuffer[3] = arrClamp.getCount();
        outputBuffer[4] = arrAbs.getCount();
    }
    // BUF: 6
    // BUF-NEXT: 4
    // BUF-NEXT: 3
    // BUF-NEXT: 2
    // BUF-NEXT: 5
}
