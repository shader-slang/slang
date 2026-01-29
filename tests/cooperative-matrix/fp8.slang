//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -output-using-type -emit-spirv-directly

// This test exercises using fp8 Cooperative Matrix type in matmuladd operation.
// This currently runs only on Vulkan, as Vulkan only allows using fp8 types in m16k32n16 shape,
// which is not supported on CUDA.

// CHECK: type: float
// CHECK-NEXT: 1
// CHECK-NEXT: 2
// CHECK-NEXT: 3
// CHECK-NEXT: 4

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer
RWStructuredBuffer<float> outputBuffer;

// 0x38 = e4m3 (1.0)
//TEST_INPUT:ubuffer(data=[0x38 0 0 0 0 0 0 0 0x3800 0 0 0 0 0 0 0  0x380000 0 0 0 0 0 0 0  0x38000000 0 0 0 0 0 0 0 ], stride=4, count=256),name=input1
ByteAddressBuffer input1;

// 0x4442403C = e4m3 (1.0, 2.0, 3.0, 4.0)
//TEST_INPUT:ubuffer(data=[0x48444038 0 0 0], stride=4, count=256),name=input2
ByteAddressBuffer input2;

using namespace linalg;

typealias FP8CoopMatA = CoopMat<FloatE4M3, MemoryScope.Subgroup, 16, 32, CoopMatMatrixUse.MatrixA>;
typealias FP8CoopMatB = CoopMat<FloatE4M3, MemoryScope.Subgroup, 32, 16, CoopMatMatrixUse.MatrixB>;
typealias FP32CoopMat = CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse.MatrixAccumulator>;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    let stride = 16;
    let matrixLayout = CoopMatMatrixLayout::RowMajor;

    let mat1 = FP8CoopMatA.Load<CoopMatMatrixLayout::RowMajor>(input1, 0, stride);
    let mat2 = FP8CoopMatB.Load<CoopMatMatrixLayout::RowMajor>(input2, 0, stride);
    FP32CoopMat result = FP32CoopMat(0.0f);
    result = coopMatMulAdd<float, false>(mat1, mat2, result);
    result.Store<CoopMatMatrixLayout::RowMajor>(outputBuffer, 0, stride);
}
