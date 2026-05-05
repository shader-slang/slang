//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_9

// CUDA fp8 (FloatE4M3) cooperative matrix multiply-add with **half** accumulator
// and output.  Targets PTX
// `mma.sync.aligned.m16n8k16.row.col.f16.e4m3.e4m3.f16` (the f16-accumulator
// variant of the fp8 mma instruction).
//
// Identity-A times B-with-first-row-[1,2,3,4,...] gives result whose first
// row is [1,2,3,4,...]; the test checks the first four output entries.

// CHECK: type: half
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4

//TEST_INPUT:ubuffer(stride=2, count=256):out,name=outputBuffer
RWStructuredBuffer<half> outputBuffer;

// e4m3 16x16 identity matrix (row-major).
//TEST_INPUT:ubuffer(data=[0x00000038 0 0 0  0x00003800 0 0 0  0x00380000 0 0 0  0x38000000 0 0 0  0 0x00000038 0 0  0 0x00003800 0 0  0 0x00380000 0 0  0 0x38000000 0 0  0 0 0x00000038 0  0 0 0x00003800 0  0 0 0x00380000 0  0 0 0x38000000 0  0 0 0 0x00000038  0 0 0 0x00003800  0 0 0 0x00380000  0 0 0 0x38000000], stride=4, count=64),name=input1
ByteAddressBuffer input1;

// e4m3 16x16 matrix B (row-major): first row = [1.0, 2.0, 3.0, 4.0, 0, ..., 0].
//TEST_INPUT:ubuffer(data=[0x48444038 0 0 0], stride=4, count=64),name=input2
ByteAddressBuffer input2;

using namespace linalg;

typealias FP8CoopMatA = CoopMat<FloatE4M3, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA>;
typealias FP8CoopMatB = CoopMat<FloatE4M3, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixB>;
typealias FP16CoopMat = CoopMat<half,      MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;

    let mat1 = FP8CoopMatA.Load<CoopMatMatrixLayout::RowMajor>(input1, 0, stride);
    let mat2 = FP8CoopMatB.Load<CoopMatMatrixLayout::RowMajor>(input2, 0, stride);
    FP16CoopMat result = FP16CoopMat(half(0.0));
    // half accumulator and output -> mma.sync.f16.e4m3.e4m3.f16 path.
    result = coopMatMulAdd<half, false>(mat1, mat2, result);
    result.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, stride);
}
