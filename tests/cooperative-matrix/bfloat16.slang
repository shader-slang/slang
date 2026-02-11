//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly

// This test exercises using bfloat16 Cooperative Matrix type in matmuladd operation.

// CHECK: type: float
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// bfloat16 (identity matrix)
//TEST_INPUT:ubuffer(data=[0x3F80 0 0 0 0 0 0 0 0x3F800000 0 0 0 0 0 0 0 0 0x3F80 0 0 0 0 0 0 0 0x3F800000 0 0 0 0 0 0 0 0 0x3F80 0 0 0 0 0 0 0 0x3F800000 0 0 0 0 0 0 0 0 0x3F80 0 0 0 0 0 0 0 0x3F800000 0 0 0 0 0 0 0 0 0x3F80 0 0 0 0 0 0 0 0x3F800000 0 0 0 0 0 0 0 0 0x3F80 0 0 0 0 0 0 0 0x3F800000 0 0 0 0 0 0 0 0 0x3F80 0 0 0 0 0 0 0 0x3F800000 0 0 0 0 0 0 0 0 0x3F80 0 0 0 0 0 0 0 0x3F800000], stride=4, count=256),name=input1
ByteAddressBuffer input1;

// bfloat (1.0, 2.0, 3.0, 4.0)
//TEST_INPUT:ubuffer(data=[0x40003F80 0x40804040 0 0], stride=4, count=256),name=input2
ByteAddressBuffer input2;

using namespace linalg;

typealias BF16CoopMatA = CoopMat<BFloat16, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixA>;
typealias BF16CoopMatB = CoopMat<BFloat16, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixB>;
typealias FP32CoopMat = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    let matrixLayout = CoopMatMatrixLayout::RowMajor;

    let mat1 = BF16CoopMatA.Load<CoopMatMatrixLayout::RowMajor>(input1, 0, stride);
    let mat2 = BF16CoopMatB.Load<CoopMatMatrixLayout::RowMajor>(input2, 0, stride);
    FP32CoopMat result = FP32CoopMat(0.0f);
    result = coopMatMulAdd<float, false>(mat1, mat2, result);
    result.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, stride);
}
