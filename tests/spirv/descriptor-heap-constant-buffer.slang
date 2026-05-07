// Regression for #11037: using `DescriptorHandle<ConstantBuffer<T>>` under
// `-capability spvDescriptorHeapEXT` crashed (debug: assert, release: segv)
// in the SPIR-V backend because `IRSPIRVLoadDescriptorFromHeap` returning an
// `IRConstantBufferType` was never lowered to a pointer before emission.
//
// Covers:
//  - ConstantBuffer with a struct element (reporter's shape)
//  - ConstantBuffer with a non-struct (matrix) element, which stresses the
//    wrapper-struct + std140 size/alignment path
//  - Mixed RW structured buffer + constant buffer through descriptor handles

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry main -stage compute -capability spvDescriptorHeapEXT

struct Uniforms
{
    uint x;
    uint y;
};

[shader("compute")]
[numthreads(1, 1, 1)]
void main(
    uniform ConstantBuffer<Uniforms>.Handle input,
    uniform ConstantBuffer<float4x4>.Handle xform,
    uniform RWStructuredBuffer<uint>.Handle output)
{
    // The struct-field and wrapped-matrix constant-buffer accesses must both
    // produce Uniform-class access chains, and the RW structured buffer must
    // still produce StorageBuffer chains.  Previously, the ConstantBuffer
    // path crashed the compiler.
    // CHECK-NOT: OpAccessChain %_ptr_Function
    // CHECK: OpAccessChain %_ptr_Uniform_uint
    // CHECK: OpAccessChain %_ptr_StorageBuffer_uint
    Uniforms u = getDescriptorFromHandle(input);
    float4x4 m = getDescriptorFromHandle(xform);
    output[0] = u.x + u.y + (uint)m[0][0];
}
