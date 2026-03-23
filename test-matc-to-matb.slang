using namespace linalg;

RWStructuredBuffer<float> outputBuffer;
ByteAddressBuffer input;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;

    let matC = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>.Load<CoopMatMatrixLayout::RowMajor>(input, 0, stride);

    // MatC(float) -> MatB(float) conversion
    let matB = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(matC);

    matB.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, stride);
}
