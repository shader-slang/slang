// Regression test for descriptor-heap sourced ByteAddressBuffer values.
// Byte-address-buffer legalization must rewrite the heap load to an equivalent
// structured-buffer heap load before SPIR-V emission.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -capability spvDescriptorHeapEXT -emit-spirv-directly
//TEST:SIMPLE(filecheck=ROV): -target spirv-asm -entry fragmentMain -stage fragment -capability spvDescriptorHeapEXT -emit-spirv-directly

// CHECK: OpCapability DescriptorHeapEXT
// CHECK: OpUntypedAccessChainKHR

// ROV: OpCapability DescriptorHeapEXT
// ROV: OpUntypedAccessChainKHR

RWStructuredBuffer<uint> output;

[numthreads(1, 1, 1)]
void computeMain()
{
    let readOnlyBuffer = DescriptorHandle<ByteAddressBuffer>(0);
    let readWriteBuffer = DescriptorHandle<RWByteAddressBuffer>(uint(1));

    let value0 = readOnlyBuffer.Load<uint>(0);
    readWriteBuffer.Store<uint>(4, value0 + 1);
    let value1 = readWriteBuffer.Load<uint>(4);
    output[0] = value1;
}

[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    let rasterOrderedBuffer =
        DescriptorHandle<RasterizerOrderedByteAddressBuffer>(uint(2));

    rasterOrderedBuffer.Store<uint>(8, 1);
    return float4(rasterOrderedBuffer.Load<uint>(8), 0, 0, 1);
}
