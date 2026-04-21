//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -Xslang -DBAB
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -Xslang -DBAB
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=CHECK_METAL):-metal -compute -output-using-type -xslang -DMETAL

//DISABLE_TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_10 -entry computeMain -Xdxc -Ibuild/dxc/include -DBAB
//DXIL: result code = 0

// CHECK-COUNT-256: A

// CHECK_METAL: type: float
// CHECK_METAL-COUNT-64: 10.0

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
#if defined(METAL)
RWStructuredBuffer<float> outputBuffer;
#elif defined(BAB)
RWByteAddressBuffer outputBuffer;
#else
RWStructuredBuffer<int32_t> outputBuffer;
#endif

using namespace linalg;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
#ifdef METAL
    var result : CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse::MatrixAccumulator>;
    result.fill(10.0);
    result.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 8);
#else
    var result : CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>;
    result.fill(10);
    result.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, 16);
#endif
}

