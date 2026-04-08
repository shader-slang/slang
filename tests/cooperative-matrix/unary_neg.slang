//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_BAB):-vk -emit-spirv-directly -Xslang -DBAB
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_BAB):-cuda -Xslang -DBAB

//DISABLE_TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_10 -entry computeMain -Xdxc -Ibuild/dxc/include -DBAB
//DXIL: result code = 0

// CHECK: -1
// CHECK-NEXT: -2
// CHECK-NEXT: -3
// CHECK-NEXT: -4

// CHECK_BAB: FFFFFFFF
// CHECK_BAB-NEXT: FFFFFFFE
// CHECK_BAB-NEXT: FFFFFFFD
// CHECK_BAB-NEXT: FFFFFFFC

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4, count=256),name=inputBuffer
ByteAddressBuffer inputBuffer;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
#if defined(BAB)
RWByteAddressBuffer outputBuffer;
#else
RWStructuredBuffer<int32_t> outputBuffer;
#endif

using namespace linalg;

typealias CoopMatType = CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 4;
    let matrixLayout = CoopMatMatrixLayout::RowMajor;

    let mat = CoopMatType.Load<CoopMatMatrixLayout::RowMajor>(inputBuffer, 0, stride);

    let result = -mat;
    result.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 4);
}
