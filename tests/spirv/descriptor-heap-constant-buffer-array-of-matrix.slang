// Regression test for #11483: a `ConstantBuffer<T>` fetched through the
// descriptor-heap untyped-pointer path (`spvDescriptorHeapEXT`) must lay out a
// nested array-of-matrix member identically to a bound `[[vk::binding]]` buffer.
//
// Before #11211 this exact `ConstantBuffer<T>.Handle(...)` pattern crashed during
// SPIR-V emission. After #11211 it emits valid SPIR-V; this test additionally pins
// the layout of the reporter's struct shape (six leading `float4x4`, a nested
// `float4x4[4]` at std140 offset 384, then a trailing `float4` at 640 — the member
// the issue reporter saw read back wrong data), guarding both the crash and the
// layout against regression.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute -capability spvDescriptorHeapEXT

struct CameraData
{
    float4x4 view;
    float4x4 proj;
    float4x4 viewProj;
    float4x4 invView;
    float4x4 invProj;
    float4x4 invViewProj;
    float4x4 shadowTransforms[4];
    float4 cascadeSplits;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform DescriptorHandle<RWStructuredBuffer<float>> output, uniform uint idx)
{
    let cb = getDescriptorFromHandle(ConstantBuffer<CameraData>.Handle(uint2(idx, 0)));
    getDescriptorFromHandle(output)[0] =
        cb.shadowTransforms[2][1][2] + cb.cascadeSplits.x + cb.view[0][0];
}

// The constant-buffer element struct must carry Block (required for the Uniform
// storage class the descriptor-heap buffer pointer uses).
// CHECK: OpName %[[CB:CameraData_std140]] "CameraData_std140"
// CHECK-DAG: OpDecorate %[[CB]] Block

// std140 member offsets matching a bound [[vk::binding]] ConstantBuffer of the same
// struct: the six leading float4x4 occupy 0..384, the float4x4[4] array starts at
// member 6 / offset 384, and the trailing vector lands at member 7 / offset 640.
// CHECK-DAG: OpMemberDecorate %[[CB]] 0 Offset 0
// CHECK-DAG: OpMemberDecorate %[[CB]] 6 Offset 384
// CHECK-DAG: OpMemberDecorate %[[CB]] 7 Offset 640

// The nested array-of-matrix carries the matching std140 strides. The array
// element stride is on the matrix array type, and the per-matrix stride is on the
// std140 matrix wrapper the array elements are laid out through.
// CHECK-DAG: OpDecorate %_arr_mat4v4float_int_4 ArrayStride 64
// CHECK-DAG: OpMemberDecorate %_Array_std140_matrix_float_4_4_4 0 MatrixStride 16

// The descriptor-heap load must produce a Uniform-class buffer pointer to the
// block, never a Function-class pointer.
// CHECK-NOT: %_ptr_Function_CameraData_std140
// CHECK: %[[CBPTR:_ptr_Uniform_CameraData_std140]] = OpTypePointer Uniform %[[CB]]
// CHECK: OpBufferPointerEXT %[[CBPTR]]
