using namespace linalg;

RWStructuredBuffer<float> outputBuffer;
ByteAddressBuffer input;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;

    let matC = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>.Load<CoopMatMatrixLayout::RowMajor>(input, 0, stride);

    // MatC(float) -> MatB(float)
    let matB = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(matC);

    let matA = CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(1.0h);
    let accum = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(0.0f);
    let result = coopMatMulAdd<float, false>(matA, matB, accum);

    result.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, stride);
}
