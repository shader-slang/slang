// Regression for #11019-family issue #11027: reading a struct field through
// a `DescriptorHandle<RWStructuredBuffer<Struct>>` produced invalid SPIR-V
// because the field-address OpAccessChain inherited `Function` storage class
// from a Generic-addressed IR pointer.  The chained pointer produced by
// `RWStructuredBufferGetElementPtr` on a descriptor-heap-sourced buffer must
// carry the StorageBuffer address space so that the subsequent field-address
// access chain inherits it correctly.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry compute_main -stage compute -capability spvDescriptorHeapEXT

struct Thing
{
    uint a;
    uint b;
};

[shader("compute")]
[numthreads(1, 1, 1)]
void compute_main(
    DescriptorHandle<RWStructuredBuffer<uint>> output,
    DescriptorHandle<RWStructuredBuffer<Thing>> things,
    DescriptorHandle<StructuredBuffer<Thing>> thingsRO)
{
    // Read-through-descriptor-handle must pick up StorageBuffer storage class
    // on every pointer in the chain.  Before the fix, the field-access chain
    // on `things[i].a` produced `OpAccessChain %_ptr_Function_uint ...` which
    // violates SPIR-V's storage-class-match rule on OpAccessChain.
    // CHECK-NOT: OpAccessChain %_ptr_Function
    // CHECK: OpAccessChain %_ptr_StorageBuffer_uint
    output[0] = things[0].a + things[0].b + thingsRO[1].a;
}
