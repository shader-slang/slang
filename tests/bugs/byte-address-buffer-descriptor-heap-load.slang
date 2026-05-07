// Regression test for issue #10440:
// ByteAddressBuffer.Load from DescriptorHandle<ByteAddressBuffer> should compile to valid SPIRV.
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -capability spvDescriptorHeapEXT

// CHECK: OpBufferPointerEXT

RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    let handle = DescriptorHandle<ByteAddressBuffer>(uint2(0, 0));
    outputBuffer[0] = handle.Load<uint>(0);
}
