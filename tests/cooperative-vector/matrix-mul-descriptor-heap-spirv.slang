// Regression test for issue #10979:
// coopVecMatMul with DescriptorHandle<ByteAddressBuffer> should compile to valid SPIRV.
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry computeMain -stage compute -capability spvDescriptorHeapEXT

// CHECK: OpBufferPointerEXT
// CHECK: OpCooperativeVectorMatrixMulNV

RWStructuredBuffer<float16_t> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadID: SV_DispatchThreadID)
{
    DescriptorHandle<RWByteAddressBuffer> input =
        DescriptorHandle<RWByteAddressBuffer>(uint2(0, 0));
    DescriptorHandle<ByteAddressBuffer> weights =
        DescriptorHandle<ByteAddressBuffer>(uint2(1, 0));

    CoopVec<float16_t, 4> layer = coopVecLoad<4, float16_t>(input, 0);

    let output = coopVecMatMul<float16_t, 4, 4>(
        layer,
        CoopVecComponentType::Float16,
        weights,
        0,
        CoopVecComponentType::Float16,
        CoopVecMatrixLayout::RowMajor,
        false,
        8);

    outputBuffer[0] = output[0];
}
