// Regression test: verifies that a struct element type still compiles successfully and
// produces correct StorageBuffer-storage-class SPIRV for the descriptor-heap buffer
// pointer (since #11483 the descriptor-heap constant buffer is StorageBuffer, not Uniform).

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute -capability spvDescriptorHeapEXT

struct MyConstants {
    float2 value;
}

[shader("compute")]
[NumThreads(1, 1, 1)]
void main(
    uniform DescriptorHandle<ConstantBuffer<MyConstants>> input,
    uniform DescriptorHandle<RWStructuredBuffer<float2>> output)
{
    output[0] = getDescriptorFromHandle(input).value;
}

// CHECK: OpName %[[CB:MyConstants_std140]] "MyConstants_std140"
// CHECK-NOT: %_ptr_Function_[[CB]]
// CHECK: OpDecorate %[[CB]] Block
// CHECK-NOT: %_ptr_Function_[[CB]]
// CHECK: %[[CB_PTR:_ptr_StorageBuffer_MyConstants_std140]] = OpTypePointer StorageBuffer %[[CB]]
// CHECK-NOT: %_ptr_Function_[[CB]]
// CHECK: OpBufferPointerEXT %[[CB_PTR]]
// CHECK: OpAccessChain %_ptr_StorageBuffer_v2float
// CHECK-NOT: %_ptr_Function_[[CB]]
