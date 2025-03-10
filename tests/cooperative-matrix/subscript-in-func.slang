//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly

// CHECK: type: float
// CHECK-NEXT: 1.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 9.000000
// CHECK-NEXT: 16.000000

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4):in,name=inputBuffer
StructuredBuffer<float> inputBuffer;

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

typealias CoopMatType = CoopMat<float, CoopMatScope::Subgroup, 2, 2, CoopMatMatrixUse::A>;

static const int stride = 4;
static const CoopMatMatrixLayout matrixLayout = CoopMatMatrixLayout::RowMajor;

void squareCoopMatElements(CoopMatType mat)
{
    for (int i = 0; i < 4; ++i)
    {
        mat[i] = mat[i] * mat[i];
    }
    mat.store(outputBuffer, 0, stride, matrixLayout);
}

[numthreads(1, 1, 1)]
void computeMain()
{
    let mat = CoopMatType.load(inputBuffer, 0, stride, matrixLayout);
    squareCoopMatElements(mat);
}
