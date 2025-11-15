//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type

// CHECK: type: float
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 9.000000
// CHECK-NEXT: 16.000000

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4, count=256):name=inputBuffer
StructuredBuffer<float> inputBuffer;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

using namespace linalg;

typealias CoopMatType = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>;

static const int stride = 16;
static const CoopMatMatrixLayout matrixLayout = CoopMatMatrixLayout::RowMajor;

void squareCoopMatElements(CoopMatType mat)
{
    for (int i = 0; i < 4; ++i)
    {
        mat[i] = mat[i] * mat[i];
    }
    mat.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, stride);
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    let mat = CoopMatType.Load<CoopMatMatrixLayout::RowMajor>(inputBuffer, 0, stride);
    squareCoopMatElements(mat);
}
