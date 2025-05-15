//TEST(compute):SIMPLE(filecheck=CHECK): -entry computeMain -stage compute -target spirv -skip-spirv-validation

// This test checks that the correct SPIRV Cooperative Matrix Operands are emitted for OpCooperativeMatrixMulAddKHR operaions
RWStructuredBuffer<uint32_t> outputBuffer1;
RWStructuredBuffer<uint32_t> outputBuffer2;
RWStructuredBuffer<uint32_t> outputBuffer3;
RWStructuredBuffer<int> outputBuffer4;
RWStructuredBuffer<uint32_t> outputBuffer5;
RWStructuredBuffer<int> outputBuffer6;

using namespace linalg;

__generic<T : __BuiltinArithmeticType>
typealias CoopMatAType = CoopMat<T, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>;

__generic<T : __BuiltinArithmeticType>
typealias CoopMatBType = CoopMat<T, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>;

__generic<T : __BuiltinArithmeticType>
typealias CoopMatCType = CoopMat<T, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>;

[numthreads(32, 1, 1)]
void computeMain()
{
    // CHECK: OpCooperativeMatrixMulAddKHR {{.*}} NoneKHR
    coopMatMulAdd<uint32_t, false>(
        CoopMatAType<uint16_t>(2),
        CoopMatBType<uint32_t>(3),
        CoopMatCType<uint16_t>(4)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer1, 0, 16);

    // CHECK: OpCooperativeMatrixMulAddKHR {{.*}} MatrixASignedComponentsKHR|MatrixBSignedComponentsKHR
    coopMatMulAdd<uint32_t, false>(
        CoopMatAType<int16_t>(2),
        CoopMatBType<int32_t>(3),
        CoopMatCType<uint16_t>(4)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer2, 0, 16);

    // CHECK: OpCooperativeMatrixMulAddKHR {{.*}} MatrixCSignedComponentsKHR
    coopMatMulAdd<uint32_t, false>(
        CoopMatAType<uint16_t>(2),
        CoopMatBType<uint32_t>(3),
        CoopMatCType<int16_t>(4)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer3, 0, 16);

    // CHECK: OpCooperativeMatrixMulAddKHR {{.*}} MatrixResultSignedComponentsKHR
    coopMatMulAdd<int, false>(
        CoopMatAType<uint16_t>(2),
        CoopMatBType<uint32_t>(3),
        CoopMatCType<uint16_t>(4)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer4, 0, 16);

    // CHECK: OpCooperativeMatrixMulAddKHR {{.*}} SaturatingAccumulationKHR
    coopMatMulAdd<uint32_t, true>(
        CoopMatAType<uint16_t>(2),
        CoopMatBType<uint32_t>(3),
        CoopMatCType<uint16_t>(4)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer5, 0, 16);

    // CHECK: OpCooperativeMatrixMulAddKHR {{.*}} MatrixASignedComponentsKHR|MatrixBSignedComponentsKHR|MatrixCSignedComponentsKHR|MatrixResultSignedComponentsKHR|SaturatingAccumulationKHR
    coopMatMulAdd<int, true>(
        CoopMatAType<int16_t>(2),
        CoopMatBType<int32_t>(3),
        CoopMatCType<int16_t>(4)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer6, 0, 16);
}


