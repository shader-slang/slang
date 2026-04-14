//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_BAB):-vk -emit-spirv-directly -Xslang -DBAB
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_BAB):-cuda -Xslang -DBAB

//DISABLE_TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_10 -entry computeMain -Xdxc -Ibuild/dxc/include -DBAB
//DXIL: result code = 0

// CHECK: 3.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 9.000000
// CHECK-NEXT: 12.000000

// CHECK_BAB: 40400000
// CHECK_BAB-NEXT: 40C00000
// CHECK_BAB-NEXT: 41100000
// CHECK_BAB-NEXT: 41400000

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=4, count=256):name=inputBuffer
#if defined(BAB)
ByteAddressBuffer inputBuffer;
#else
StructuredBuffer<float> inputBuffer;
#endif

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
#if defined(BAB)
RWByteAddressBuffer outputBuffer;
#else
RWStructuredBuffer<float> outputBuffer;
#endif

using namespace linalg;

typealias CoopMatType = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>;

static let stride = 16;
//static let matrixLayout = CoopMatMatrixLayout::RowMajor;
static const CoopMatMatrixLayout matrixLayout = CoopMatMatrixLayout::RowMajor;

void processCoopMat(CoopMatType mat)
{
    // XXX: hmmm, some error when matrixLAyout is static let
    (mat * 3.0).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, stride);
}

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    let mat = CoopMatType.Load<CoopMatMatrixLayout::RowMajor>(inputBuffer, 0, stride);
    processCoopMat(mat);
}
