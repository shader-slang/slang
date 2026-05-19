// Regression test for #11037: segmentation fault (assertion failure) when
// ConstantBuffer<T>.Handle is used with spvDescriptorHeapEXT.
//
// The SPIRV legalize pass was not lowering ConstantBuffer<T> to a Uniform pointer,
// so the SPIRV emitter's getDescriptorHeapBufferStorageClass hit an assertion.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute -capability spvDescriptorHeapEXT

struct Uniforms {
    uint x;
}

[shader("compute")]
[NumThreads(1, 1, 1)]
void main(
    uniform DescriptorHandle<ConstantBuffer<Uniforms>> input,
    uniform DescriptorHandle<RWStructuredBuffer<uint>> output)
{
    output[0] = getDescriptorFromHandle(input).x;
}

// The struct must carry Block decoration (required for Uniform storage class).
// CHECK: OpDecorate {{.*}}Uniforms{{.*}} Block

// The pointer to the uniform block must be in Uniform storage class.
// CHECK: OpTypePointer Uniform

// The descriptor-heap access must produce a buffer pointer.
// CHECK: OpBufferPointerEXT

// Field access must use Uniform storage class — not Function.
// CHECK-NOT: OpAccessChain %_ptr_Function
// CHECK: OpAccessChain %_ptr_Uniform
