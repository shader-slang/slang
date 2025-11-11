//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK-VK):-vk -output-using-type -emit-spirv-directly
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK-CUDA):-cuda -output-using-type

// CHECK-VK: type: float
// CHECK-VK-COUNT-256: 241.0

// CHECK-VK: type: int32_t
// CHECK-VK-COUNT-256: 481

// CHECK-VK: type: int32_t
// CHECK-VK-COUNT-256: -2147483169

// CHECK-VK: type: int32_t
// CHECK-VK-COUNT-128: 481

// CHECK-VK: type: int32_t
// CHECK-VK-COUNT-128: -2147483169

// CHECK-CUDA: type: float
// CHECK-CUDA-COUNT-256: 241.0

// CHECK-CUDA: type: int32_t
// CHECK-CUDA-COUNT-256: 241

// CHECK-CUDA: type: int32_t
// CHECK-CUDA-COUNT-256: -2147483409

// CHECK-CUDA: type: int32_t
// CHECK-CUDA-COUNT-256: 2147483647

// CHECK-CUDA: type: int32_t
// CHECK-CUDA-COUNT-256: 241

// CHECK-CUDA: type: int32_t
// CHECK-CUDA-COUNT-256: -2147483409

// CHECK-CUDA: type: int32_t
// CHECK-CUDA-COUNT-256: 2147483647

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer1
RWStructuredBuffer<float> outputBuffer1;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer2
RWStructuredBuffer<int32_t> outputBuffer2;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer3
RWStructuredBuffer<int32_t> outputBuffer3;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer4
RWStructuredBuffer<int32_t> outputBuffer4;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer5
RWStructuredBuffer<int32_t> outputBuffer5;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer6
RWStructuredBuffer<int32_t> outputBuffer6;

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer7
RWStructuredBuffer<int32_t> outputBuffer7;

using namespace linalg;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
    // ( 3.0 * 5.0 ) * 16 + 1.0 = 241.0
    coopMatMulAdd<float, false>(
            CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(3.0),
            CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(5.0),
            CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(1.0)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer1, 0, 16);

    __target_switch
    {
    case spirv:
        // ( 3.0 * 5.0 ) * 32 + 1.0 = 481
        coopMatMulAdd<int32_t, false>(
                CoopMat<int8_t, MemoryScope.Subgroup, 16, 32, CoopMatMatrixUse::MatrixA >(3),
                CoopMat<int8_t, MemoryScope.Subgroup, 32, 16, CoopMatMatrixUse::MatrixB >(5),
                CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(1)
        ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer2, 0, 16);

        // m16n16k32 test
        // overflow test
        // ( 3 * 5 ) * 32 + 0x7FFFFFFF = 0xF0 + 0x7FFFFFFF = 0x800000EF = -2147483409
        coopMatMulAdd<int32_t, false>(
                CoopMat<int8_t, MemoryScope.Subgroup, 16, 32, CoopMatMatrixUse::MatrixA >(3),
                CoopMat<int8_t, MemoryScope.Subgroup, 32, 16, CoopMatMatrixUse::MatrixB >(5),
                CoopMat<int32_t, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(0x7FFFFFFF)
        ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer3, 0, 16);

        // saturation test - Not supported on at least 4090 GPU with 580 driver.

        // m16n8k32 test
        // ( 3 * 5 ) * 32 + 1 = 481
        coopMatMulAdd<int32_t, false>(
                CoopMat<int8_t, MemoryScope.Subgroup, 16, 32, CoopMatMatrixUse::MatrixA >(3),
                CoopMat<int8_t, MemoryScope.Subgroup, 32, 8, CoopMatMatrixUse::MatrixB >(5),
                CoopMat<int32_t, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse::MatrixAccumulator>(1)
        ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer4, 0, 8);

        // overflow test
        // ( 3 * 5 ) * 32 + 0x7FFFFFFF = 0x1E0 + 0x7FFFFFFF = 0x800001DF = -2147483169
        coopMatMulAdd<int32_t, false>(
                CoopMat<int8_t, MemoryScope.Subgroup, 16, 32, CoopMatMatrixUse::MatrixA >(3),
                CoopMat<int8_t, MemoryScope.Subgroup, 32, 8, CoopMatMatrixUse::MatrixB >(5),
                CoopMat<int32_t, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse::MatrixAccumulator>(0x7FFFFFFF)
        ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer5, 0, 8);

    case cuda:
        // m32n8k16 test
        // ( 3 * 5 ) * 16 + 1 = 241
        coopMatMulAdd<int32_t, false>(
                CoopMat<int8_t, MemoryScope.Subgroup, 32, 16, CoopMatMatrixUse::MatrixA >(3),
                CoopMat<int8_t, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse::MatrixB >(5),
                CoopMat<int32_t, MemoryScope.Subgroup, 32, 8, CoopMatMatrixUse::MatrixAccumulator>(1)
        ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer2, 0, 8);

        // overflow test
        // ( 3 * 5 ) * 16 + 0x7FFFFFFF = 0xF0 + 0x7FFFFFFF = 0x800000EF = -2147483409
        coopMatMulAdd<int32_t, false>(
                CoopMat<int8_t, MemoryScope.Subgroup, 32, 16, CoopMatMatrixUse::MatrixA >(3),
                CoopMat<int8_t, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse::MatrixB >(5),
                CoopMat<int32_t, MemoryScope.Subgroup, 32, 8, CoopMatMatrixUse::MatrixAccumulator>(0x7FFFFFFF)
        ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer3, 0, 8);

        // saturation test
        // ( 3 * 5 ) * 16 + 0x7FFFFFFF = 2147483887, and then saturated to [-2147483648, 2147483647] = 2147483647
        coopMatMulAdd<int32_t, true>(
                CoopMat<int8_t, MemoryScope.Subgroup, 32, 16, CoopMatMatrixUse::MatrixA >(3),
                CoopMat<int8_t, MemoryScope.Subgroup, 16, 8, CoopMatMatrixUse::MatrixB >(5),
                CoopMat<int32_t, MemoryScope.Subgroup, 32, 8, CoopMatMatrixUse::MatrixAccumulator>(0x7FFFFFFF)
        ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer4, 0, 8);

        //m8n32k16 test
        // ( 3 * 5 ) * 16 + 1 = 241
        coopMatMulAdd<int32_t, false>(
                CoopMat<int8_t, MemoryScope.Subgroup, 8, 16, CoopMatMatrixUse::MatrixA >(3),
                CoopMat<int8_t, MemoryScope.Subgroup, 16, 32, CoopMatMatrixUse::MatrixB >(5),
                CoopMat<int32_t, MemoryScope.Subgroup, 8, 32, CoopMatMatrixUse::MatrixAccumulator>(1)
        ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer5, 0, 32);

        // overflow test
        // ( 3 * 5 ) * 16 + 0x7FFFFFFF = 0xF0 + 0x7FFFFFFF = 0x800000EF = -2147483409
        coopMatMulAdd<int32_t, false>(
                CoopMat<int8_t, MemoryScope.Subgroup, 8, 16, CoopMatMatrixUse::MatrixA >(3),
                CoopMat<int8_t, MemoryScope.Subgroup, 16, 32, CoopMatMatrixUse::MatrixB >(5),
                CoopMat<int32_t, MemoryScope.Subgroup, 8, 32, CoopMatMatrixUse::MatrixAccumulator>(0x7FFFFFFF)
        ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer6, 0, 32);

        // saturation test
        // ( 3 * 5 ) * 16 + 0x7FFFFFFF = 2147483887, and then saturated to [-2147483648, 2147483647] = 2147483647
        coopMatMulAdd<int32_t, true>(
                CoopMat<int8_t, MemoryScope.Subgroup, 8, 16, CoopMatMatrixUse::MatrixA >(3),
                CoopMat<int8_t, MemoryScope.Subgroup, 16, 32, CoopMatMatrixUse::MatrixB >(5),
                CoopMat<int32_t, MemoryScope.Subgroup, 8, 32, CoopMatMatrixUse::MatrixAccumulator>(0x7FFFFFFF)
        ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer7, 0, 32);
    }
}
