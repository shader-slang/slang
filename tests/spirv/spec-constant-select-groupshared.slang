//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage compute

// Test that a ternary/select on a specialization constant used as a groupshared
// array size compiles successfully to SPIR-V using OpSpecConstantOp Select.

// CHECK: %[[FOO:[0-9A-Za-z_]+]] = OpSpecConstant %uint 4
// CHECK: %[[CMP:[0-9A-Za-z_]+]] = OpSpecConstantOp %bool UGreaterThan %[[FOO]] %{{[0-9A-Za-z_]+}}
// CHECK: %[[SIZE:[0-9A-Za-z_]+]] = OpSpecConstantOp %uint Select %[[CMP]] %[[FOO]] %{{[0-9A-Za-z_]+}}
// CHECK: %[[SIZE_INT:[0-9A-Za-z_]+]] = OpSpecConstantOp %int UConvert %[[SIZE]]
// CHECK: %[[ARR_TYPE:[0-9A-Za-z_]+]] = OpTypeArray %float %[[SIZE_INT]]
// CHECK: %[[PT_TYPE:[0-9A-Za-z_]+]] = OpTypePointer Workgroup %[[ARR_TYPE]]
// CHECK: OpVariable %[[PT_TYPE]] Workgroup

[vk::specialization_constant]
const uint FOO = 4;

groupshared float arr[FOO > 0 ? FOO : 1];

//TEST_INPUT:ubuffer(data=[0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

[shader("compute")]
[numthreads(4, 1, 1)]
void main(uint tid : SV_GroupIndex)
{
    arr[tid] = float(tid);
    GroupMemoryBarrier();
    if (tid == 0)
        outputBuffer[0] = arr[0];
}
