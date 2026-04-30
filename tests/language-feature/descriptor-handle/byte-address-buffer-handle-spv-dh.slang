// Regression test for #10979 and #10440: DescriptorHandle<ByteAddressBuffer> with spvDescriptorHeapEXT
// crashed (missing case in getEquivalentStructuredBuffer for IRSPIRVLoadDescriptorFromHeap).
//TEST:SIMPLE(filecheck=CHECK): -target spirv -capability spvDescriptorHeapEXT

// CHECK: OpDecorate {{.*}} BuiltIn ResourceHeapEXT
// CHECK: OpCooperativeVectorLoadNV
// CHECK: OpCooperativeVectorMatrixMulNV

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 threadID: SV_DispatchThreadID)
{
    DescriptorHandle<RWByteAddressBuffer> input = DescriptorHandle<RWByteAddressBuffer>(uint2(0, 0));
    DescriptorHandle<ByteAddressBuffer> weights = DescriptorHandle<ByteAddressBuffer>(uint2(1, 0));
    DescriptorHandle<RWStructuredBuffer<CoopVec<float16_t, 4>>> outputBuf =
        DescriptorHandle<RWStructuredBuffer<CoopVec<float16_t, 4>>>(uint2(2, 0));

    // coopVecLoad via DescriptorHandle<RWByteAddressBuffer> — exercises #10979 coopVecLoad path
    CoopVec<float16_t, 4> vec = coopVecLoad<4, float16_t>(input, 0);

    // coopVecMatMul via DescriptorHandle<ByteAddressBuffer> — exercises #10979 matmul path
    let result = coopVecMatMul<float16_t, 4, 4>(
        vec,
        CoopVecComponentType::Float16,
        weights,
        0,
        CoopVecComponentType::Float16,
        CoopVecMatrixLayout::RowMajor,
        false,
        8);

    outputBuf[0] = result;
}
