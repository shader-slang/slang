// Regression test: a struct element type still compiles and produces correct SPIRV for the
// descriptor-heap buffer pointer. Since #12226 the descriptor-heap constant buffer is a
// uniform-buffer descriptor whose data pointer is an untyped Uniform pointer.

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
// CHECK: OpDecorate %[[CB]] Block
// CHECK: OpTypeBufferEXT Uniform
// CHECK: %[[UNIFORM_PTR:_ptr_Uniform]] = OpTypeUntypedPointerKHR Uniform{{$}}
// CHECK: OpBufferPointerEXT %[[UNIFORM_PTR]]
// CHECK: OpUntypedAccessChainKHR %[[UNIFORM_PTR]] %[[CB]]
// CHECK-NOT: OpTypePointer StorageBuffer %[[CB]]
