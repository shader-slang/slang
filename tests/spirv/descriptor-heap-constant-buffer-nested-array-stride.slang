// Regression test for #11483: a `ConstantBuffer<T>` fetched through the
// descriptor-heap path (`spvDescriptorHeapEXT`) read back wrong data for any nested-array
// member, because the descriptor-heap buffer pointer was emitted as a typed pointer that
// carried no pointer-type `ArrayStride`, so addressing a nested array through it computed an
// undefined offset; only the leading scalar at offset 0 was correct.
//
// Since #12226 the descriptor-heap constant buffer is fetched as a uniform-buffer descriptor
// (`OpTypeBufferEXT Uniform`) whose data pointer is an untyped Uniform pointer, and members
// are addressed with `OpUntypedAccessChainKHR` naming the pointee aggregate. The stride comes
// from ordinary array-type `ArrayStride` decorations (logical addressing), so nested arrays
// address correctly with no pointer-type `ArrayStride` and the descriptor keeps its
// uniform-buffer kind. The reporter hit this under scalar layout, so both the default std140
// path and `-fvk-use-scalar-layout` (`_natural`) are pinned below.

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

// --- default std140 layout ---
// The nested arrays carry ordinary array-type ArrayStride decorations (std140 rounds the
// float[4] element stride up to 16); no pointer-type ArrayStride is emitted.
// CHECK-DAG: OpDecorate %_arr_float_int_4 ArrayStride 16
// CHECK-DAG: OpDecorate %_arr_v4float_int_4 ArrayStride 16
// CHECK-DAG: OpDecorate %_arr_mat4v4float_int_4 ArrayStride 64
// The descriptor-heap ConstantBuffer is a uniform-buffer descriptor whose data pointer is an
// untyped Uniform pointer, and its members are addressed with untyped access chains naming the
// pointee aggregate as the Base Type.
// CHECK: OpTypeBufferEXT Uniform
// CHECK: %[[UNIFORM_PTR:_ptr_Uniform]] = OpTypeUntypedPointerKHR Uniform{{$}}
// CHECK: OpBufferPointerEXT %[[UNIFORM_PTR]]
// CHECK: OpUntypedAccessChainKHR %[[UNIFORM_PTR]] %CameraData_std140
// A regression back to a typed StorageBuffer constant-buffer pointer must not reappear.
// CHECK-NOT: OpTypePointer StorageBuffer %CameraData_std140

// --- scalar layout (-fvk-use-scalar-layout): the configuration the #11483 reporter used ---
// CHECKSCALAR-DAG: OpDecorate %_arr_float_int_4 ArrayStride 4
// CHECKSCALAR-DAG: OpDecorate %_arr_v4float_int_4 ArrayStride 16
// CHECKSCALAR-DAG: OpDecorate %_arr_mat4v4float_int_4 ArrayStride 64
// CHECKSCALAR: OpTypeBufferEXT Uniform
// CHECKSCALAR: %[[UNIFORM_PTRN:_ptr_Uniform]] = OpTypeUntypedPointerKHR Uniform{{$}}
// CHECKSCALAR: OpBufferPointerEXT %[[UNIFORM_PTRN]]
// CHECKSCALAR: OpUntypedAccessChainKHR %[[UNIFORM_PTRN]] %CameraData_natural
// CHECKSCALAR-NOT: OpTypePointer StorageBuffer %CameraData_natural
