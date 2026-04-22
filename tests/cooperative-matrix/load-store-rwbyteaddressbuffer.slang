//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly -Xslang -DRWBAB
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -Xslang -DRWBAB
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=CHECK_METAL):-metal -compute -output-using-type -xslang -DMETAL

//DISABLE_TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_10 -entry computeMain -Xdxc -Ibuild/dxc/include
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
#if defined(RWBAB)
RWByteAddressBuffer inputBufferInt;
#else
ByteAddressBuffer inputBufferInt;
#endif

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBufferInt
RWByteAddressBuffer outputBufferInt;

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0 5.0 6.0 7.0 8.0], stride=4, count=256):name=inputBufferFloat
StructuredBuffer<float> inputBufferFloat;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBufferFloat
RWStructuredBuffer<float> outputBufferFloat;

using namespace linalg;

[numthreads(32, 1, 1)]
void computeMain()
{
#ifndef METAL
    let stride = 16;
    let mat = coopMatLoad<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(inputBufferInt, 0, stride);
    mat.Store<CoopMatMatrixLayout.RowMajor>(outputBufferInt, 0, 16);
#else
    let stride = 8;
    let mat = coopMatLoad<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(inputBufferFloat, 0, stride);
    mat.Store<CoopMatMatrixLayout.RowMajor>(outputBufferFloat, 0, 8);
#endif
}
