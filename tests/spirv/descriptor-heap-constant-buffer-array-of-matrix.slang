// Regression test for #11483: a `ConstantBuffer<T>` fetched through the
// descriptor-heap untyped-pointer path (`spvDescriptorHeapEXT`) must lay out a
// nested array-of-matrix member identically to a bound `[[vk::binding]]` buffer.
//
// Before #11211 this exact `ConstantBuffer<T>.Handle(...)` pattern crashed during
// SPIR-V emission. After #11211 it emits valid SPIR-V; this test additionally pins
// the layout of a nested `float4x4[N]` member (the member the issue reporter saw
// read back wrong data), guarding both the crash and the layout against regression.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute -capability spvDescriptorHeapEXT

struct CameraData
{
    float intensity;
    float4x4 shadowTransforms[4];
    float4 tint;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uniform DescriptorHandle<RWStructuredBuffer<float>> output, uniform uint idx)
{
    let cb = getDescriptorFromHandle(ConstantBuffer<CameraData>.Handle(uint2(idx, 0)));
    getDescriptorFromHandle(output)[0] =
        cb.shadowTransforms[2][1][2] + cb.tint.x + cb.intensity;
}

// The constant-buffer element struct must carry Block (required for the Uniform
// storage class the descriptor-heap buffer pointer uses).
// CHECK: OpName %[[CB:CameraData_std140]] "CameraData_std140"
// CHECK-DAG: OpDecorate %[[CB]] Block

// std140 member offsets: scalar at 0, the float4x4[4] array at 16, the trailing
// vector after the 256-byte array at 272.
// CHECK-DAG: OpMemberDecorate %[[CB]] 0 Offset 0
// CHECK-DAG: OpMemberDecorate %[[CB]] 1 Offset 16
// CHECK-DAG: OpMemberDecorate %[[CB]] 2 Offset 272

// The nested array-of-matrix must carry the matching std140 strides.
// CHECK-DAG: OpDecorate %{{[A-Za-z0-9_]+}} ArrayStride 64
// CHECK-DAG: OpMemberDecorate %{{[A-Za-z0-9_]+}} 0 MatrixStride 16

// The descriptor-heap load must produce a Uniform-class buffer pointer to the
// block, never a Function-class pointer.
// CHECK-NOT: %_ptr_Function_CameraData_std140
// CHECK: %[[CBPTR:_ptr_Uniform_CameraData_std140]] = OpTypePointer Uniform %[[CB]]
// CHECK: OpBufferPointerEXT %[[CBPTR]]
