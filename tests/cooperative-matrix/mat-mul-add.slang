//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -emit-spirv-directly -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_BAB):-vk -emit-spirv-directly -Xslang -DBAB
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK_CUDA):-cuda -output-using-type
//TEST(compute, metal):COMPARE_COMPUTE(filecheck-buffer=CHECK_METAL):-metal -compute -output-using-type -xslang -DMETAL

//DISABLE_TEST(compute):SIMPLE(filecheck=DXIL):-target dxil -profile cs_6_10 -entry computeMain -Xdxc -Ibuild/dxc/include -DBAB
//DXIL: result code = 0

// CHECK: type: float
// CHECK-COUNT-256: 241.0
// CHECK_BAB-COUNT-256: 43710000

// CHECK_METAL: type: float
// CHECK_METAL-COUNT-64: 121.0

// CHECK: type: int32_t
// CHECK-COUNT-256: 481
// CHECK_BAB-COUNT-256: 1E1

// CHECK: type: int32_t
// CHECK-COUNT-256: -2147483169
// CHECK_BAB-COUNT-256: 800001DF

// CHECK: type: int32_t
// CHECK-COUNT-128: 481
// CHECK_BAB-COUNT-128: 1E1

// CHECK: type: int32_t
// CHECK-COUNT-128: -2147483169
// CHECK_BAB-COUNT-128: 800001DF


// CHECK_CUDA: type: float
// CHECK_CUDA-COUNT-256: 241.0

// CHECK_CUDA: type: int32_t
// CHECK_CUDA-COUNT-256: 241

// CHECK_CUDA: type: int32_t
// CHECK_CUDA-COUNT-256: -2147483409

// CHECK_CUDA: type: int32_t
// CHECK_CUDA-COUNT-256: 2147483647

// CHECK_CUDA: type: int32_t
// CHECK_CUDA-COUNT-256: 241

// CHECK_CUDA: type: int32_t
// CHECK_CUDA-COUNT-256: -2147483409

// CHECK_CUDA: type: int32_t
// CHECK_CUDA-COUNT-256: 2147483647

//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer1
//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer2
//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer3
//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer4
//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer5
//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer6
//TEST_INPUT:ubuffer(stride=4, count=256):out,name=outputBuffer7

#if defined(BAB)
RWByteAddressBuffer outputBuffer1;
RWByteAddressBuffer outputBuffer2;
RWByteAddressBuffer outputBuffer3;
RWByteAddressBuffer outputBuffer4;
RWByteAddressBuffer outputBuffer5;
RWByteAddressBuffer outputBuffer6;
RWByteAddressBuffer outputBuffer7;
#else
RWStructuredBuffer<float> outputBuffer1;
RWStructuredBuffer<int32_t> outputBuffer2;
RWStructuredBuffer<int32_t> outputBuffer3;
RWStructuredBuffer<int32_t> outputBuffer4;
RWStructuredBuffer<int32_t> outputBuffer5;
RWStructuredBuffer<int32_t> outputBuffer6;
RWStructuredBuffer<int32_t> outputBuffer7;
#endif

using namespace linalg;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain()
{
#ifdef METAL
    // ( 3.0 * 5.0 ) * 8 + 1.0 = 121.0
    coopMatMulAdd<float, false>(
            CoopMat<half, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse::MatrixA>(3.0),
            CoopMat<half, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse::MatrixB>(5.0),
            CoopMat<float, MemoryScope.Subgroup, 8, 8, CoopMatMatrixUse::MatrixAccumulator>(1.0)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer1, 0, 8);
#else
    // ( 3.0 * 5.0 ) * 16 + 1.0 = 241.0
    coopMatMulAdd<float, false>(
            CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixA>(3.0),
            CoopMat<half, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixB>(5.0),
            CoopMat<float, MemoryScope.Subgroup, 16, 16, CoopMatMatrixUse::MatrixAccumulator>(1.0)
    ).Store<CoopMatMatrixLayout::RowMajor>(outputBuffer1, 0, 16);

    __target_switch
    {
    case hlsl:
        break;
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
#endif
}
