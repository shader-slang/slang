//TEST:SIMPLE(filecheck=SPIRV): -target spirv -stage compute -entry main
//SPIRV: OpCapability BFloat16CooperativeMatrixKHR

uniform bfloat16_t* inputBuffer;
RWStructuredBuffer<float32_t> outputBuffer;

using namespace linalg;

//SPIRV: [[COOPMATA_TYPE:%.*]] = OpTypeCooperativeMatrixKHR %bfloat16 %int_3 %int_16 %int_16 %int_0
//SPIRV: [[MATA_VAL:%.*]] = OpConstantComposite [[COOPMATA_TYPE:%.*]] %bfloat16_0x1_8p_1
typealias CoopMatAType = CoopMat<bfloat16_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>;
//SPIRV: [[COOPMATB_TYPE:%.*]] = OpTypeCooperativeMatrixKHR %bfloat16 %int_3 %int_16 %int_16 %int_1
typealias CoopMatBType = CoopMat<bfloat16_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>;
//SPIRV: [[COOPMATC_TYPE:%.*]] = OpTypeCooperativeMatrixKHR %float %int_3 %int_16 %int_16 %int_2
//SPIRV: [[MATC_VAL:%.*]] = OpConstantComposite [[COOPMATC_TYPE]] %float_1
typealias CoopMatCType = CoopMat<float32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>;

[numthreads(32, 1, 1)]
void main()
{
    bfloat16_t* ptrIn = inputBuffer;
    let matA = CoopMatAType(3.0bf);
    // [[MATB_VAL:%.*]] = OpCooperativeMatrixLoadKHR [[COOPMATB_TYPE]] {{.*}} %int_0 %uint_16 Aligned 16
    let matB = coopMatLoad<bfloat16_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixB, CoopMatMatrixLayout.RowMajor>(ptrIn, 0, 16);
    let matC = CoopMatCType(1.0f);

    // OpCooperativeMatrixMulAddKHR [[COOPMATC_TYPE]] [[MATA_VAL]] [[MATB_VAL]] [[MATC_VAL]] NoneKHR
    let result = coopMatMulAdd<float, false>(matA, matB, matC);
    result.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 16);
}
