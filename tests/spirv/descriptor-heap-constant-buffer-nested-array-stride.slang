// Regression test for #11483: a `ConstantBuffer<T>` fetched through the
// descriptor-heap untyped-pointer path (`spvDescriptorHeapEXT`) read back wrong data
// for any nested-array member, because the descriptor-heap buffer pointer was emitted
// in the Uniform storage class. Uniform pointer types carry no pointer-type
// `ArrayStride` (Uniform uses logical addressing), so addressing a nested array
// through the `OpBufferPointerEXT`-derived pointer computed an undefined offset; only
// the leading scalar at offset 0 was correct. A bound `[[vk::binding]]` ConstantBuffer
// and a `StructuredBuffer<T>` fetched the same way are unaffected.
//
// The fix retypes the descriptor-heap ConstantBuffer buffer pointer into the
// StorageBuffer storage class (matching the StructuredBuffer-via-heap path), so the
// buffer pointer and its nested-array element pointer types receive the pointer-type
// `ArrayStride` decorations the driver needs. The reporter hit this under scalar
// layout, so both the default std140 path and `-fvk-use-scalar-layout` (`_natural`)
// are pinned below.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute -capability spvDescriptorHeapEXT
//TEST:SIMPLE(filecheck=CHECKSCALAR): -target spirv-asm -entry main -stage compute -capability spvDescriptorHeapEXT -fvk-use-scalar-layout

struct CameraData
{
    float scalar;
    float vals[4];
    float4 vecs[4];
    float4x4 mats[4];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform DescriptorHandle<RWStructuredBuffer<float>> output, uniform uint idx)
{
    let cb = getDescriptorFromHandle(ConstantBuffer<CameraData>.Handle(uint2(idx, 0)));
    getDescriptorFromHandle(output)[0] =
        cb.vals[2] + cb.vecs[1].x + cb.mats[3][1][2] + cb.scalar;
}

// Checks are ordered to match SPIR-V section order: annotations (OpDecorate) before
// types (OpTypePointer) before the function body (OpBufferPointerEXT).
//
// --- default std140 layout ---
// The nested-array element pointer types must carry pointer-type ArrayStride so the
// buffer-pointer addressing of vals[i] / vecs[i] / mats[i] is well-defined.
// CHECK-DAG: OpDecorate %_ptr_StorageBuffer__arr_float_int_4 ArrayStride 16
// CHECK-DAG: OpDecorate %_ptr_StorageBuffer__arr_v4float_int_4 ArrayStride 64
// CHECK-DAG: OpDecorate %_ptr_StorageBuffer__arr_mat4v4float_int_4 ArrayStride 256
// The descriptor-heap ConstantBuffer must be fetched as a StorageBuffer-class buffer
// pointer (a regression back to Uniform would rename this type and fail the match),
// and that pointer must feed the OpBufferPointerEXT heap fetch.
// CHECK: %[[CBPTR:_ptr_StorageBuffer_CameraData_std140]] = OpTypePointer StorageBuffer %CameraData_std140
// CHECK: OpBufferPointerEXT %[[CBPTR]]

// --- scalar layout (-fvk-use-scalar-layout): the configuration the #11483 reporter used ---
// CHECKSCALAR-DAG: OpDecorate %_ptr_StorageBuffer__arr_float_int_4 ArrayStride 16
// CHECKSCALAR-DAG: OpDecorate %_ptr_StorageBuffer__arr_v4float_int_4 ArrayStride 64
// CHECKSCALAR-DAG: OpDecorate %_ptr_StorageBuffer__arr_mat4v4float_int_4 ArrayStride 256
// CHECKSCALAR: %[[CBPTRN:_ptr_StorageBuffer_CameraData_natural]] = OpTypePointer StorageBuffer %CameraData_natural
// CHECKSCALAR: OpBufferPointerEXT %[[CBPTRN]]
