//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -Xslang -DRWSB
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -Xslang -DBAB
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -Xslang -DRWSB
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -Xslang -DBAB

//TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_10 -entry computeMain -dxc-path dxc-sm610/ -Xdxc -Idxc-sm610/include -DBAB
//DXIL: result code = 0

// CHECK: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8

//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8], stride=4, count=256),name=buf
#if defined(RWSB)
RWStructuredBuffer<int32_t> inputBuffer;
#elif defined(BAB)
ByteAddressBuffer inputBuffer;
#else
StructuredBuffer<int32_t> inputBuffer;
#endif

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
#if defined(BAB)
RWByteAddressBuffer outputBuffer;
#else
RWStructuredBuffer<int32_t> outputBuffer;
#endif

using namespace linalg;

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    let mat = coopMatLoad<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(inputBuffer, 0, stride);
    mat.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);
}
