//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_BAB):-vk -emit-spirv-directly -Xslang -DBAB

//TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_10 -entry computeMain -dxc-path dxc-sm610/ -Xdxc -Idxc-sm610/include -DBAB
//DXIL: result code = 0

// CHECK: 2.000000
// CHECK-NEXT: 4.000000
// CHECK-NEXT: 6.000000
// CHECK-NEXT: 8.000000

// CHECK_BAB: 40000000
// CHECK_BAB-NEXT: 40800000
// CHECK_BAB-NEXT: 40C00000
// CHECK_BAB-NEXT: 41000000

//TEST_INPUT:ubuffer(data=[0 0 0 0], stride=4, count=256):out,name=outputBuffer
#if defined(BAB)
RWByteAddressBuffer outputBuffer;
#else
RWStructuredBuffer<float> outputBuffer;
#endif

//TEST_INPUT:ubuffer(data=[1 2 3 4], stride=4, count=256),name=input
ByteAddressBuffer input;

using namespace linalg;

[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    let matrixLayout = CoopMatMatrixLayout::RowMajor;

    let intMat = CoopMat<int, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>.Load<CoopMatMatrixLayout::RowMajor>(input, 0, stride);
    let floatMat = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(intMat);
    let uintMat = CoopMat<uint, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(intMat);
    let halfMat = CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(uintMat);
    let floatMat2 = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(halfMat);

    let result = floatMat + floatMat2;
    result.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, stride);
}
