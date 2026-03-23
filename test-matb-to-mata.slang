using namespace linalg;

RWStructuredBuffer<float> outputBuffer;
ByteAddressBuffer input;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;

    let matB = CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>.Load<CoopMatMatrixLayout::RowMajor>(input, 0, stride);

    // MatB -> MatA conversion (this is the part we want to inspect)
    let matA = CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(matB);

    let accum = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(0.0f);
    let result = coopMatMulAdd<float, false>(matA, matB, accum);

    result.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, stride);
}
