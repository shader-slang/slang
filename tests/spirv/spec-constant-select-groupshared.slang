//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry main -stage compute

// Test that a ternary/select on a specialization constant used as a groupshared
// array size compiles successfully to SPIR-V using OpSpecConstantOp Select.

// CHECK: %[[FOO:[0-9A-Za-z_]+]] = OpSpecConstant %int 4
// CHECK: %[[CMP:[0-9A-Za-z_]+]] = OpSpecConstantOp %bool SGreaterThan %[[FOO]] %{{[0-9A-Za-z_]+}}
// CHECK: %[[SIZE:[0-9A-Za-z_]+]] = OpSpecConstantOp %int Select %[[CMP]] %[[FOO]] %{{[0-9A-Za-z_]+}}
// CHECK: %[[ARR_TYPE:[0-9A-Za-z_]+]] = OpTypeArray %float %[[SIZE]]
// CHECK: %[[PT_TYPE:[0-9A-Za-z_]+]] = OpTypePointer Workgroup %[[ARR_TYPE]]
// CHECK: OpVariable %[[PT_TYPE]] Workgroup

// GLSL: const int [[SIZE:_S[0-9]+]] = FOO_0 > 0 ? FOO_0 : 1;
// GLSL: shared float {{ *}}arr_0{{\[}}[[SIZE]]{{\]}};
// WGSL: var<workgroup> arr_0 : array<f32, select(i32(1), FOO_0, (FOO_0 > i32(0)))>;

[vk::specialization_constant]
const int FOO = 4;

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
