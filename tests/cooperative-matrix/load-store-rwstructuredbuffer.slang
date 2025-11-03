//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DRWSB
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -Xslang -DRWSB

// CHECK: type: int32_t
// CHECK-NEXT: 1
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
#else
StructuredBuffer<int32_t> inputBuffer;
#endif

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<int32_t> outputBuffer;

using namespace linalg;

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    let mat = coopMatLoad<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(inputBuffer, 0, stride);
    mat.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);
}
