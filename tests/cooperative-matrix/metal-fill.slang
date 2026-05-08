//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=CHECK):-metal -compute -output-using-type

// CHECK: type: float
// CHECK-COUNT-64: 10.0

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

using namespace linalg;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    var result : CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse::MatrixAccumulator>;
    result.fill(10.0);
    result.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0u, 8u);
}
