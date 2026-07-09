// Regression test for #11483: a `ConstantBuffer<T>` fetched through the
// descriptor-heap untyped-pointer path (`spvDescriptorHeapEXT`), where T has a
// nested `float4x4[4]` struct member, must emit a stable constant-buffer block
// layout under both the default std140 and `-fvk-use-scalar-layout` paths.
//
// Scope: this is a COMPILE + EMITTED-LAYOUT test (`-target spirv-asm`). It guards
//   (a) the SPIR-V emission crash #11211 fixed — a regression would fail to compile;
//   (b) the descriptor-heap block layout (member offsets / array + matrix strides);
//   (c) that the descriptor-heap ConstantBuffer is fetched as a StorageBuffer-class
//       buffer pointer. (#11483 was miscompiled because it was emitted Uniform-class,
//       and a Uniform pointer carries no pointer-type ArrayStride, so nested-array
//       reads through the buffer pointer were undefined; the dedicated pointer-stride
//       regression lives in descriptor-heap-constant-buffer-nested-array-stride.slang.)
// It does NOT execute on the GPU, so the runtime readback half of #11483 still needs
// hardware verification.
//
// The reporter hit this under scalar layout, so both the default std140 path and the
// `-fvk-use-scalar-layout` (`_natural`) path the reporter actually used are pinned.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=CHECKSCALAR): -target spirv-asm -entry main -stage compute -capability spvDescriptorHeapEXT -fvk-use-scalar-layout

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

// --- default std140 layout ---
// The constant-buffer element struct must carry Block (required for the StorageBuffer
// storage class the descriptor-heap buffer pointer uses).
// CHECK: OpName %[[CB:CameraData_std140]] "CameraData_std140"
// CHECK-DAG: OpDecorate %[[CB]] Block
// std140 member offsets: the six leading float4x4 occupy 0..384, the float4x4[4]
// array starts at member 6 / offset 384, and the trailing vector lands at offset 640.
// CHECK-DAG: OpMemberDecorate %[[CB]] 0 Offset 0
// CHECK-DAG: OpMemberDecorate %[[CB]] 6 Offset 384
// CHECK-DAG: OpMemberDecorate %[[CB]] 7 Offset 640
// The nested array-of-matrix strides land on the matrix array type and the std140
// matrix wrapper the array elements are laid out through (matrices are RowMajor).
// CHECK-DAG: OpDecorate %_arr_mat4v4float_int_4 ArrayStride 64
// CHECK-DAG: OpMemberDecorate %_Array_std140_matrix_float_4_4_4 0 MatrixStride 16
// CHECK-DAG: OpMemberDecorate %_Array_std140_matrix_float_4_4_4 0 RowMajor
// The descriptor-heap load must produce a StorageBuffer-class buffer pointer to the
// block (#11483: a Uniform-class pointer carries no pointer-type ArrayStride), never a
// Function-class pointer (which would mean a copy lowering, not the heap path).
// CHECK-NOT: %_ptr_Function_[[CB]]
// CHECK: %[[CBPTR:_ptr_StorageBuffer_CameraData_std140]] = OpTypePointer StorageBuffer %[[CB]]
// CHECK: OpBufferPointerEXT %[[CBPTR]]

// --- scalar layout (-fvk-use-scalar-layout): the configuration the #11483 reporter used ---
// Same absolute offsets/strides as std140 for this struct; only the type names differ
// (`_natural` instead of `_std140`).
// CHECKSCALAR: OpName %[[CBN:CameraData_natural]] "CameraData_natural"
// CHECKSCALAR-DAG: OpDecorate %[[CBN]] Block
// CHECKSCALAR-DAG: OpMemberDecorate %[[CBN]] 0 Offset 0
// CHECKSCALAR-DAG: OpMemberDecorate %[[CBN]] 6 Offset 384
// CHECKSCALAR-DAG: OpMemberDecorate %[[CBN]] 7 Offset 640
// CHECKSCALAR-DAG: OpDecorate %_arr_mat4v4float_int_4 ArrayStride 64
// CHECKSCALAR-DAG: OpMemberDecorate %_Array_natural_matrix_float_4_4_4 0 MatrixStride 16
// CHECKSCALAR-DAG: OpMemberDecorate %_Array_natural_matrix_float_4_4_4 0 RowMajor
// CHECKSCALAR-NOT: %_ptr_Function_[[CBN]]
// CHECKSCALAR: %[[CBNPTR:_ptr_StorageBuffer_CameraData_natural]] = OpTypePointer StorageBuffer %[[CBN]]
// CHECKSCALAR: OpBufferPointerEXT %[[CBNPTR]]
