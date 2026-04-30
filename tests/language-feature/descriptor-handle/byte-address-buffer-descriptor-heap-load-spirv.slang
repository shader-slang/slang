// Regression test for #10440: DescriptorHandle<ByteAddressBuffer>.Load<T> with spvDescriptorHeapEXT
// crashed ("Unhandled global inst in spirv-emit: ByteAddressBuffer").
//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability spvDescriptorHeapEXT

// CHECK: OpDecorate {{.*}} BuiltIn ResourceHeapEXT

RWStructuredBuffer<uint> output;
[numthreads(1,1,1)]
void computeMain()
{
    let handle = DescriptorHandle<ByteAddressBuffer>(uint2(0, 0));
    output[0] = handle.Load<uint>(0);

    // RWByteAddressBuffer variant reported in #10440
    let rwHandle = DescriptorHandle<RWByteAddressBuffer>(uint2(1, 0));
    output[1] = rwHandle.Load<uint>(4);
}
