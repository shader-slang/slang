//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -emit-spirv-directly

// Check that SPIR-V type instructions are memoized by both their encoded
// operands and layout-sensitive decorations emitted separately from the type.

struct FixedArrayData
{
    float2 v[2];
};

struct RuntimeArrayData
{
    float f;
    float2 v[];
};

struct PointerMatrixData
{
    row_major float3x2 m1;
    column_major float3x2 m2;
};

GLSLShaderStorageBuffer<FixedArrayData, Std140DataLayout> fixed140;
GLSLShaderStorageBuffer<FixedArrayData, Std430DataLayout> fixed430;

GLSLShaderStorageBuffer<RuntimeArrayData, Std140DataLayout> runtime140;
GLSLShaderStorageBuffer<RuntimeArrayData, Std430DataLayout> runtime430;

GLSLShaderStorageBuffer<PointerMatrixData, Std140DataLayout> pointerData;

cbuffer PhysicalPointers
{
    LayoutPtr<float2, Std140DataLayout> physical140;
    LayoutPtr<float2, Std430DataLayout> physical430;
    LayoutPtr<float2[2], Std140DataLayout> physicalArray140;
    LayoutPtr<float2[2], Std430DataLayout> physicalArray430;
}

RWStructuredBuffer<float> outputBuffer;

// OpTypeArray: same element type and length, different ArrayStride decorations.
// CHECK-DAG: OpDecorate %[[ARRAY_140:_arr_v2float[A-Za-z0-9_]*]] ArrayStride 16
// CHECK-DAG: OpDecorate %[[ARRAY_430:_arr_v2float[A-Za-z0-9_]*]] ArrayStride 8
// CHECK-DAG: %[[V2:[A-Za-z0-9_]+]] = OpTypeVector %float 2
// CHECK-DAG: %[[ARRAY_140]] = OpTypeArray %[[V2]] %{{[A-Za-z0-9_]+}}
// CHECK-DAG: %[[ARRAY_430]] = OpTypeArray %[[V2]] %{{[A-Za-z0-9_]+}}

// OpTypeRuntimeArray: same element type, different ArrayStride decorations.
// CHECK-DAG: OpDecorate %[[RUNTIME_140:_runtimearr_v2float[A-Za-z0-9_]*]] ArrayStride 16
// CHECK-DAG: OpDecorate %[[RUNTIME_430:_runtimearr_v2float[A-Za-z0-9_]*]] ArrayStride 8
// CHECK-DAG: %[[RUNTIME_140]] = OpTypeRuntimeArray %[[V2]]
// CHECK-DAG: %[[RUNTIME_430]] = OpTypeRuntimeArray %[[V2]]

// OpTypePointer: same storage class and pointee type, different ArrayStride decorations.
// CHECK-DAG: OpDecorate %[[M1_PTR:_ptr_StorageBuffer_mat3v2float[A-Za-z0-9_]*]] ArrayStride 48
// CHECK-DAG: OpDecorate %[[M2_PTR:_ptr_StorageBuffer_mat3v2float[A-Za-z0-9_]*]] ArrayStride 32
// CHECK-DAG: %[[MATRIX:[A-Za-z0-9_]+]] = OpTypeMatrix %[[V2]] 3
// CHECK-DAG: %[[M1_PTR]] = OpTypePointer StorageBuffer %[[MATRIX]]
// CHECK-DAG: %[[M2_PTR]] = OpTypePointer StorageBuffer %[[MATRIX]]

// OpTypePointer PhysicalStorageBuffer: same storage class and pointee type,
// different ArrayStride decorations.
// CHECK-DAG: OpDecorate %[[PHYSICAL_140:_ptr_PhysicalStorageBuffer_v2float[A-Za-z0-9_]*]] ArrayStride 16
// CHECK-DAG: OpDecorate %[[PHYSICAL_430:_ptr_PhysicalStorageBuffer_v2float[A-Za-z0-9_]*]] ArrayStride 8
// CHECK-DAG: %[[PHYSICAL_140]] = OpTypePointer PhysicalStorageBuffer %[[V2]]
// CHECK-DAG: %[[PHYSICAL_430]] = OpTypePointer PhysicalStorageBuffer %[[V2]]

// OpTypePointer to a sized array uses the whole array object stride, not the
// array element stride.
// CHECK-DAG: OpDecorate %[[PHYSICAL_ARRAY_140:_ptr_PhysicalStorageBuffer__arr_v2float[A-Za-z0-9_]*]] ArrayStride 32
// CHECK-DAG: OpDecorate %[[PHYSICAL_ARRAY_430:_ptr_PhysicalStorageBuffer__arr_v2float[A-Za-z0-9_]*]] ArrayStride 16
// CHECK-DAG: %[[PHYSICAL_ARRAY_140]] = OpTypePointer PhysicalStorageBuffer %[[ARRAY_140]]
// CHECK-DAG: %[[PHYSICAL_ARRAY_430]] = OpTypePointer PhysicalStorageBuffer %[[ARRAY_430]]

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    outputBuffer[0] =
        fixed140.v[0].x +
        fixed430.v[0].x +
        runtime140.f +
        runtime140.v[0].x +
        runtime430.f +
        runtime430.v[0].x +
        pointerData.m1[0][0] +
        pointerData.m2[0][0] +
        physical140[0].x +
        physical430[0].x +
        physicalArray140[1][0].x +
        physicalArray430[1][0].x;
}
