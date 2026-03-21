//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_BAB):-vk -emit-spirv-directly -Xslang -DBAB
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_BAB):-cuda -Xslang -DBAB

//TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_10 -entry computeMain -dxc-path dxc-sm610/ -Xdxc -Idxc-sm610/include -DBAB
//DXIL: result code = 0

// CHECK: 1.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 9.000000
// CHECK-NEXT: 16.000000

// CHECK_BAB: 3F800000
// CHECK_BAB-NEXT: 40800000
// CHECK_BAB-NEXT: 41100000
// CHECK_BAB-NEXT: 41800000

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4, count=256):name=inputBuffer
//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer

#if defined(BAB)
ByteAddressBuffer inputBuffer;
RWByteAddressBuffer outputBuffer;
#else
StructuredBuffer<float> inputBuffer;
RWStructuredBuffer<float> outputBuffer;
#endif

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
