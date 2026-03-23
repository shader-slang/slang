using namespace linalg;

RWByteAddressBuffer outputBuffer;
ByteAddressBuffer input;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;

    let matB = CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>.Load<CoopMatMatrixLayout::RowMajor>(input, 0, stride);

    // MatB -> MatA conversion
    let matA = CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(matB);

    matA.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, stride);
}
