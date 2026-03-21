//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -Xslang -DBAB
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -Xslang -DBAB

//TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_10 -entry computeMain -dxc-path dxc-sm610/ -Xdxc -Idxc-sm610/include -DBAB
//DXIL: result code = 0

// CHECK: 2
// CHECK-NEXT: 4
// CHECK: 7
// CHECK-NEXT: B

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
    CoopMatType mat;
    mat[0] = 2;
    mat[1] = mat[0]+2;
    mat[2] = mat[1]+3;
    mat[3] = mat[2]+4;
    mat.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 16);
}
