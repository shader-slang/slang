//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_BAB):-vk -emit-spirv-directly -Xslang -DBAB
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_BAB):-cuda -Xslang -DBAB
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=CHECK_METAL):-metal -compute -output-using-type -xslang -DMETAL

//DISABLE_TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_10 -entry computeMain -Xdxc -Ibuild/dxc/include -DBAB
//DXIL: result code = 0

// CHECK: 1.000000
// CHECK-NEXT: 2.000000
// CHECK-NEXT: 3.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 5.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 7.000000
// CHECK-NEXT: 8.000000

// CHECK_METAL: 1.000000
// CHECK_METAL-NEXT: 2.000000
// CHECK_METAL-NEXT: 3.000000
// CHECK_METAL-NEXT: 4.000000
// CHECK_METAL-NEXT: 5.000000
// CHECK_METAL-NEXT: 6.000000
// CHECK_METAL-NEXT: 7.000000
// CHECK_METAL-NEXT: 8.000000

// CHECK_BAB: 3F800000
// CHECK_BAB-NEXT: 40000000
// CHECK_BAB-NEXT: 40400000
// CHECK_BAB-NEXT: 40800000
// CHECK_BAB-NEXT: 40A00000
// CHECK_BAB-NEXT: 40C00000
// CHECK_BAB-NEXT: 40E00000
// CHECK_BAB-NEXT: 41000000

//TEST_INPUT:ubuffer(data=[1.0 2.0 3.0 4.0], stride=256),name=input1
ByteAddressBuffer input1;

//TEST_INPUT:ubuffer(data=[5.0 6.0 7.0 8.0], stride=256),name=input2
ByteAddressBuffer input2;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
#if defined(BAB)
RWByteAddressBuffer outputBuffer;
#else
RWStructuredBuffer<float> outputBuffer;
#endif

using namespace linalg;

#ifdef METAL
typealias CoopMatType = CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse::MatrixAccumulator>;
#else
typealias CoopMatType = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>;
#endif

[numthreads(32, 1, 1)]
void computeMain()
{
#ifdef METAL
    let stride = 8;
#else
    let stride = 16;
#endif
    let matrixLayout = CoopMatMatrixLayout::RowMajor;

    CoopMatType coopMatArray[2];
    coopMatArray[0] = CoopMatType.Load<CoopMatMatrixLayout::RowMajor>(input1, 0, stride);
    coopMatArray[1] = CoopMatType.Load<CoopMatMatrixLayout::RowMajor>(input2, 0, stride);

    coopMatArray[0].Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, stride);
    coopMatArray[1].Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 4, stride);
}
