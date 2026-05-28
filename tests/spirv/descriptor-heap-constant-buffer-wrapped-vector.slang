// Regression test: verifies that an explicitly wrapped element type still compiles
// successfully and produces correct Uniform-storage-class SPIRV.

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
// CHECK: %[[CB_PTR:_ptr_Uniform_MyConstants_std140]] = OpTypePointer Uniform %[[CB]]
// CHECK-NOT: %_ptr_Function_[[CB]]
// CHECK: OpBufferPointerEXT %[[CB_PTR]]
// CHECK: OpAccessChain %_ptr_Uniform_v2float
// CHECK-NOT: %_ptr_Function_[[CB]]
