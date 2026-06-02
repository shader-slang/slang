// Regression test for descriptor-heap sourced ByteAddressBuffer values.
// Byte-address-buffer legalization must rewrite the heap load to an equivalent
// structured-buffer heap load before SPIR-V emission.

//TEST:COMPARE_COMPUTE(filecheck-buffer=RUNTIME): -vk -shaderobj -output-using-type -render-feature bindless -emit-spirv-directly -Xslang -DRUNTIME
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -capability spvDescriptorHeapEXT -emit-spirv-directly
//TEST:SIMPLE(filecheck=ROV): -target spirv-asm -entry fragmentMain -stage fragment -capability spvDescriptorHeapEXT -emit-spirv-directly

// RUNTIME: type: uint
// RUNTIME-NEXT: 4
// RUNTIME-NEXT: 8

// CHECK: OpCapability DescriptorHeapEXT
// CHECK: OpUntypedAccessChainKHR

// ROV: OpCapability DescriptorHeapEXT
// ROV: OpUntypedAccessChainKHR

//TEST_INPUT:ubuffer(data=[0 0], stride=4):out,name=output
RWStructuredBuffer<uint> output;

//TEST_INPUT: set readOnlyBuffer = ubuffer(data=[4 99], stride=4)
uniform ByteAddressBuffer.Handle readOnlyBuffer;

//TEST_INPUT: set readWriteBuffer = ubuffer(data=[0 0], stride=4)
uniform RWByteAddressBuffer.Handle readWriteBuffer;

uniform RasterizerOrderedByteAddressBuffer.Handle rasterOrderedBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    let value0 = readOnlyBuffer.Load<uint>(0);
    readWriteBuffer.Store<uint>(4, value0 * 2);

    output[0] = value0;
    output[1] = readWriteBuffer.Load<uint>(4);
}

[shader("fragment")]
float4 fragmentMain() : SV_Target
{
    rasterOrderedBuffer.Store<uint>(8, 1);
    return float4(rasterOrderedBuffer.Load<uint>(8), 0, 0, 1);
}
