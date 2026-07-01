//TEST:SIMPLE(filecheck=CHECK):-target spirv -entry computeMain -stage compute -emit-spirv-directly

// Check that structured-buffer element pointers preserve layout-derived
// ArrayStride decorations, including the default StructuredBuffer layout.

// CHECK-DAG: OpDecorate %[[PTR_140:_ptr_StorageBuffer_float[A-Za-z0-9_]*]] ArrayStride 16
// CHECK-DAG: OpDecorate %[[PTR_430:_ptr_StorageBuffer_float[A-Za-z0-9_]*]] ArrayStride 4
// CHECK-DAG: OpDecorate %[[PTR_DEFAULT:_ptr_StorageBuffer_v3float[A-Za-z0-9_]*]] ArrayStride 16
// CHECK-DAG: %[[V3:[A-Za-z0-9_]+]] = OpTypeVector %float 3
// CHECK-DAG: %[[PTR_140]] = OpTypePointer StorageBuffer %float
// CHECK-DAG: %[[PTR_430]] = OpTypePointer StorageBuffer %float
// CHECK-DAG: %[[PTR_DEFAULT]] = OpTypePointer StorageBuffer %[[V3]]
// CHECK: OpPtrAccessChain %[[PTR_140]]
// CHECK: OpPtrAccessChain %[[PTR_430]]
// CHECK: OpPtrAccessChain %[[PTR_DEFAULT]]

StructuredBuffer<float, Std140DataLayout> input140;
StructuredBuffer<float, Std430DataLayout> input430;
StructuredBuffer<float3> inputDefault;
RWStructuredBuffer<float> output;

float readNext<L:IBufferDataLayout>(StructuredBuffer<float, L> buffer)
{
    let pointer = __getStructuredBufferElementPtr(buffer, 0);
    return pointer[1];
}

float readDefaultNext(StructuredBuffer<float3> buffer)
{
    let pointer = __getStructuredBufferElementPtr(buffer, 0);
    return pointer[1].x;
}

[numthreads(1, 1, 1)]
void computeMain()
{
    output[0] = readNext(input140);
    output[1] = readNext(input430);
    output[2] = readDefaultNext(inputDefault);
}
