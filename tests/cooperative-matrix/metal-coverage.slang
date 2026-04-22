//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -compute -output-using-type -xslang -DMETAL

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outStructuredBuffer
RWStructuredBuffer<float> outStructuredBuffer;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outBAB
RWStructuredBuffer<float> outBAB;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outPointer
uniform float* outPointer;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outGroupsharedSame
RWStructuredBuffer<float> outGroupsharedSame;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outPtrSame
RWStructuredBuffer<float> outPtrSame;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outGroupsharedCross
RWStructuredBuffer<float> outGroupsharedCross;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outPtrCross
RWStructuredBuffer<float> outPtrCross;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outMMA
RWStructuredBuffer<float> outMMA;

//TEST_INPUT:ubuffer(data=[5.0 5.0 5.0 5.0 5.0 5.0 5.0 5.0], stride=4, count=256):name=inputBAB
ByteAddressBuffer inputBAB;

using namespace linalg;

typealias CMat = CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator>;
static const uint kStride = 8u;

groupshared float[256] gsFloat;
groupshared uint4[64] gsUint4;

#pragma warning(disable:41017)

[numthreads(32, 1, 1)]
[shader("compute")]
void computeMain()
{
    // --- 1. fill + Store to RWStructuredBuffer<T> ---
    var mat : CMat;
    mat.fill(5.0);
    mat.Store<CoopMatMatrixLayout.RowMajor>(outStructuredBuffer, 0u, kStride);
    // CHECK: type: float
    // CHECK-COUNT-8: 5.000000

    // --- 2. Load from ByteAddressBuffer + Store to RWStructuredBuffer ---
    let matBAB = CMat.Load<CoopMatMatrixLayout.RowMajor>(inputBAB, 0u, kStride);
    matBAB.Store<CoopMatMatrixLayout.RowMajor>(outBAB, 0u, kStride);
    // CHECK: type: float
    // CHECK-COUNT-8: 5.000000

    // --- 3. Store via T* pointer (hex: 40A00000 = 5.0f) ---
    mat.Store<CoopMatMatrixLayout.RowMajor>(outPointer, 0u, kStride);
    // CHECK-COUNT-8: 40A00000

    // --- 4. Store/Load via groupshared T[V] (same type) ---
    mat.Store<CoopMatMatrixLayout.RowMajor>(gsFloat, 0u, kStride);
    let matGS = CMat.Load<CoopMatMatrixLayout.RowMajor>(gsFloat, 0u, kStride);
    matGS.Store<CoopMatMatrixLayout.RowMajor>(outGroupsharedSame, 0u, kStride);
    // CHECK: type: float
    // CHECK-COUNT-8: 5.000000

    // --- 5. Store/Load via Ptr<T, GroupShared> (same type) ---
    Ptr<float, Access::ReadWrite, AddressSpace::GroupShared> ptrGS = __getAddress(gsFloat[0]);
    mat.Store<CoopMatMatrixLayout.RowMajor>(ptrGS, kStride);
    let matPtrGS = CMat.Load<CoopMatMatrixLayout.RowMajor>(ptrGS, kStride);
    matPtrGS.Store<CoopMatMatrixLayout.RowMajor>(outPtrSame, 0u, kStride);
    // CHECK: type: float
    // CHECK-COUNT-8: 5.000000

    // --- 6. Store/Load via groupshared U[V] (cross-type, explicit generics) ---
    mat.Store<CoopMatMatrixLayout.RowMajor, uint4, 64>(gsUint4, 0u, kStride);
    let matCrossGS = CMat.Load<CoopMatMatrixLayout.RowMajor, uint4, 64>(gsUint4, 0u, kStride);
    matCrossGS.Store<CoopMatMatrixLayout.RowMajor>(outGroupsharedCross, 0u, kStride);
    // CHECK: type: float
    // CHECK-COUNT-8: 5.000000

    // --- 7. Store/Load via Ptr<U, GroupShared> (cross-type) ---
    Ptr<uint4, Access::ReadWrite, AddressSpace::GroupShared> ptrCrossGS = __getAddress(gsUint4[0]);
    mat.Store<CoopMatMatrixLayout.RowMajor, uint4>(ptrCrossGS, kStride);
    let matCrossPtrGS = CMat.Load<CoopMatMatrixLayout.RowMajor, uint4>(ptrCrossGS, kStride);
    matCrossPtrGS.Store<CoopMatMatrixLayout.RowMajor>(outPtrCross, 0u, kStride);
    // CHECK: type: float
    // CHECK-COUNT-8: 5.000000

    // --- 8. coopMatMulAdd: (2*3)*8 + 1 = 49 ---
    coopMatMulAdd<float, false>(
        CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixA>(2.0),
        CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixB>(3.0),
        CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator>(1.0)
    ).Store<CoopMatMatrixLayout.RowMajor>(outMMA, 0u, kStride);
    // CHECK: type: float
    // CHECK-COUNT-8: 49.000000
}
