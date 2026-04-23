//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -Xslang -DRWSB
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -Xslang -DBAB
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -Xslang -DRWSB
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -Xslang -DBAB
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=CHECK_METAL):-metal -compute -output-using-type -Xslang -DMETAL

//DISABLE_TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_10 -entry computeMain -Xdxc -Ibuild/dxc/include -DBAB
//DXIL: result code = 0

// CHECK: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4
// CHECK-NEXT: 5
// CHECK-NEXT: 6
// CHECK-NEXT: 7
// CHECK-NEXT: 8

// CHECK_METAL: 1.0
// CHECK_METAL-NEXT: 2.0
// CHECK_METAL-NEXT: 3.0
// CHECK_METAL-NEXT: 4.0
// CHECK_METAL-NEXT: 5.0
// CHECK_METAL-NEXT: 6.0
// CHECK_METAL-NEXT: 7.0
// CHECK_METAL-NEXT: 8.0

//TEST_INPUT:ubuffer(data=[1 2 3 4 5 6 7 8], stride=4, count=256):name=inputBufferInt
#if defined(RWSB)
RWStructuredBuffer<int32_t> inputBufferInt;
#elif defined(BAB)
ByteAddressBuffer inputBufferInt;
#else
StructuredBuffer<int32_t> inputBufferInt;
#endif

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBufferInt
#if defined(BAB)
RWByteAddressBuffer outputBufferInt;
#else
RWStructuredBuffer<int32_t> outputBufferInt;
#endif

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0], stride=4, count=256):name=inputBufferFloat
#if defined(RWSB)
RWStructuredBuffer<float> inputBufferFloat;
#elif defined(BAB)
ByteAddressBuffer inputBufferFloat;
#else
StructuredBuffer<float> inputBufferFloat;
#endif

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBufferFloat
#if defined(BAB)
RWByteAddressBuffer outputBufferFloat;
#else
RWStructuredBuffer<float> outputBufferFloat;
#endif

using namespace linalg;

[numthreads(32, 1, 1)]
void computeMain()
{
#ifndef METAL
    let stride = 16;
    let mat = coopMatLoad<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(inputBufferInt, 0, stride);
    mat.Store<CoopMatMatrixLayout.RowMajor>(outputBufferInt, 0, stride);
#else
    let stride = 8;
    let mat = coopMatLoad<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(inputBufferFloat, 0, stride);
    mat.Store<CoopMatMatrixLayout.RowMajor>(outputBufferFloat, 0, stride);
#endif
}
