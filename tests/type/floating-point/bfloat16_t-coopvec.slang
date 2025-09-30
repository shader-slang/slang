//TEST:SIMPLE(filecheck=SPIRV): -target spirv -stage compute -entry main
//SPIRV: OpCapability BFloat16CooperativeMatrixKHR

using namespace linalg;

ByteAddressBuffer input;
ByteAddressBuffer matrix;
RWByteAddressBuffer outputBuffer;
RWStructuredBuffer<bfloat16_t> resultBuffer;

[numthreads(32, 1, 1)]
void main()
{
    // SPIRV: [[BF16_COOPVEC_T:%.*]] = OpTypeCooperativeVectorNV %bfloat16 %int_15
    // SPIRV: [[BF16_VEC15:%.*]] = OpCooperativeVectorLoadNV [[BF16_COOPVEC_T]] {{%.*}} %int_0 None
    let vec = coopVecLoad<15, bfloat16_t>(input);
    // SPIRV: OpCooperativeVectorStoreNV
    vec.store(outputBuffer, 128);
    // SPIRV: [[F16_VEC5:%.*]] = OpCooperativeVectorMatrixMulNV
    let matmul = coopVecMatMul<half, 5, 15>(
        vec,
        CoopVecComponentType::Float16,
        matrix,
        0,
        CoopVecComponentType::Float16,
        CoopVecMatrixLayout::RowMajor,
        false,
        4
    );
    let result = CoopVec<bfloat16_t, 5>(matmul);
    for(int i = 0; i < result.getCount(); ++i)
    {
        resultBuffer[i] = result[i];
    }
}
