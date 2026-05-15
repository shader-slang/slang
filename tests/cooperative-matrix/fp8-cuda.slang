//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_9

// CUDA fp8 (FloatE4M3) cooperative matrix multiply-add test using the
// m16n8k16 shape (i.e. CoopMat element shape 16x16, which lowers to 2x
// mma.sync.m16n8k16).  Vulkan's fp8 cooperative matrix support requires
// the m16k32n16 shape; see `fp8.slang` for the SPIR-V variant.
//
// Identity-A times B-with-first-row-[1,2,3,4,...] gives result whose first
// row is [1,2,3,4,...]; the test checks the first four output entries.

// CHECK: type: float
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// e4m3 16x16 identity matrix (row-major):  byte (i, i) = 0x38 (= e4m3 of 1.0).
// Each row is 16 bytes = 4 uint32; the diagonal element lands in uint32
// (i/4) of row i, byte-position (i%4).
//TEST_INPUT:ubuffer(data=[0x00000038 0 0 0  0x00003800 0 0 0  0x00380000 0 0 0  0x38000000 0 0 0  0 0x00000038 0 0  0 0x00003800 0 0  0 0x00380000 0 0  0 0x38000000 0 0  0 0 0x00000038 0  0 0 0x00003800 0  0 0 0x00380000 0  0 0 0x38000000 0  0 0 0 0x00000038  0 0 0 0x00003800  0 0 0 0x00380000  0 0 0 0x38000000], stride=4, count=64),name=input1
ByteAddressBuffer input1;

// e4m3 16x16 matrix B (row-major): first row = [1.0, 2.0, 3.0, 4.0, 0, ..., 0],
// rest zero.  e4m3 encoding: 1.0 -> 0x38, 2.0 -> 0x40, 3.0 -> 0x44, 4.0 -> 0x48.
//TEST_INPUT:ubuffer(data=[0x48444038 0 0 0], stride=4, count=64),name=input2
ByteAddressBuffer input2;

using namespace linalg;

typealias FP8CoopMatA = CoopMat<FloatE4M3, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA>;
typealias FP8CoopMatB = CoopMat<FloatE4M3, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixB>;
typealias FP32CoopMat = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;

    let mat1 = FP8CoopMatA.Load<CoopMatMatrixLayout::RowMajor>(input1, 0, stride);
    let mat2 = FP8CoopMatB.Load<CoopMatMatrixLayout::RowMajor>(input2, 0, stride);
    FP32CoopMat result = FP32CoopMat(0.0f);
    result = coopMatMulAdd<float, false>(mat1, mat2, result);
    result.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, stride);
}
