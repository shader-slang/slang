//TEST:SIMPLE(filecheck=CHECK): -target spirv -entry computeMain -stage compute

// This test exercises conversions between CoopMat types with both different
// element types and different CoopMatMatrixUse values.
// For example, converting from (float, MatrixAccumulator) to (half, MatrixA).

// CHECK-DAG: OpCapability CooperativeMatrixConversionsNV
// CHECK-DAG: OpFConvert

using namespace linalg;

typealias FloatAccum = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;
typealias HalfMatrixA = CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA>;
typealias HalfMatrixB = CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixB>;
typealias FloatMatrixA = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA>;

uniform half* output;
uniform float* f32output;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    // Create an accumulator matrix with float elements
    FloatAccum accumMat;
    accumMat.fill(1.0f);

    // Convert from (float, Accumulator) to (half, MatrixA)
    HalfMatrixA matA = HalfMatrixA(accumMat);

    // Convert from (half, MatrixA) to (half, MatrixB)
    HalfMatrixB matB = HalfMatrixB(matA);

    // Convert from (half, MatrixB) to (float, MatrixA)
    FloatMatrixA matA2 = FloatMatrixA(matB);

    matA.Store<linalg.CoopMatMatrixLayout.RowMajor>(output, 0u, 64u);
    matB.Store<linalg.CoopMatMatrixLayout.RowMajor>(output, 0u, 64u);
    matA2.Store<linalg.CoopMatMatrixLayout.RowMajor>(f32output, 0u, 64u);
}
