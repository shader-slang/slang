//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_BAB):-vk -emit-spirv-directly -Xslang -DBAB
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_BAB):-cuda -Xslang -DBAB
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=CHECK_METAL):-metal -compute -output-using-type -Xslang -DMETAL

//DISABLE_TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_10 -entry computeMain -Xdxc -Ibuild/dxc/include -DBAB
//DXIL: result code = 0

// CHECK: 1.00
// CHECK-NEXT: 2.00
// CHECK-NEXT: 3.00
// CHECK-NEXT: 4.00
// CHECK-NEXT: 5.00
// CHECK-NEXT: 6.00
// CHECK-NEXT: 7.00
// CHECK-NEXT: 8.00
// CHECK-NEXT: 9.00
// CHECK-NEXT: 10.00
// CHECK-NEXT: 11.00
// CHECK-NEXT: 12.00
// CHECK-NEXT: 13.00
// CHECK-NEXT: 14.00
// CHECK-NEXT: 15.00
// CHECK-NEXT: 16.00

// CHECK_METAL: 1.00
// CHECK_METAL-NEXT: 2.00
// CHECK_METAL-NEXT: 3.00
// CHECK_METAL-NEXT: 4.00
// CHECK_METAL-NEXT: 5.00
// CHECK_METAL-NEXT: 6.00
// CHECK_METAL-NEXT: 7.00
// CHECK_METAL-NEXT: 8.00
// CHECK_METAL-NEXT: 9.00
// CHECK_METAL-NEXT: 10.00
// CHECK_METAL-NEXT: 11.00
// CHECK_METAL-NEXT: 12.00
// CHECK_METAL-NEXT: 13.00
// CHECK_METAL-NEXT: 14.00
// CHECK_METAL-NEXT: 15.00
// CHECK_METAL-NEXT: 16.00

// CHECK_BAB: 40003C00
// CHECK_BAB-NEXT: 44004200
// CHECK_BAB-NEXT: 46004500
// CHECK_BAB-NEXT: 48004700
// CHECK_BAB-NEXT: 49004880
// CHECK_BAB-NEXT: 4A004980
// CHECK_BAB-NEXT: 4B004A80
// CHECK_BAB-NEXT: 4C004B80

//TEST_INPUT:ubuffer(data=[1.0h 2.0h 3.0h 4.0h 5.0h 6.0h 7.0h 8.0h 9.0h 10.0h 11.0h 12.0h 13.0h 14.0h 15.0h 16.0h], stride=2, count=256):name=input
RWByteAddressBuffer input;

//TEST_INPUT:ubuffer(stride=2, count=256):out,name=outputBuffer
#if defined(BAB)
RWByteAddressBuffer outputBuffer;
#else
RWStructuredBuffer<half> outputBuffer;
#endif

using namespace linalg;

#ifdef METAL
typealias CoopMatType = CoopMat<half, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator>;
#else
typealias CoopMatType = CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;
#endif

[numthreads(32, 1, 1)]
void computeMain()
{
#ifdef METAL
    let stride = 8;
    let mat = coopMatLoad<half, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(input, 0, stride);
#else
    let stride = 16;
    let mat = coopMatLoad<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator, CoopMatMatrixLayout.RowMajor>(input, 0, stride);
#endif

    mat.Store<CoopMatMatrixLayout.RowMajor>(outputBuffer, 0, stride);
}
