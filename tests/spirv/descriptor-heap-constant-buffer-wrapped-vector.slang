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

// CHECK: OpDecorate {{.*}}MyConstants{{.*}} Block
// CHECK: OpTypePointer Uniform
// CHECK: OpBufferPointerEXT
// CHECK-NOT: OpAccessChain %_ptr_Function
// CHECK: OpAccessChain %_ptr_Uniform
