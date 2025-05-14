//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-via-glsl
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -dx12 -use-dxil -profile sm_6_5 -shaderobj
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute  -shaderobj -xslang -DCUDA

//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly -xslang -DUSE_GLSL_SYNTAX -allow-glsl

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

#if defined(USE_GLSL_SYNTAX)
#define __partitionedInclusiveSum subgroupPartitionedInclusiveAddNV
#define __partitionedInclusiveProduct subgroupPartitionedInclusiveMulNV
#define __partitionedExclusiveSum subgroupPartitionedExclusiveAddNV
#define __partitionedExclusiveProduct subgroupPartitionedExclusiveMulNV
#else
#define __partitionedInclusiveSum WaveMultiPrefixInclusiveSum
#define __partitionedInclusiveProduct WaveMultiPrefixInclusiveProduct
#define __partitionedExclusiveSum WaveMultiPrefixExclusiveSum
#define __partitionedExclusiveProduct WaveMultiPrefixExclusiveProduct
#endif

static uint partitionedIndex = 0;
static uint gProductValue = 0;

__generic<T : __BuiltinArithmeticType>
bool test1SumProduct(uint4 mask)
{
    let productValue = T(gProductValue);

    return true
        & (__partitionedInclusiveSum(T(1), mask) == T(partitionedIndex + 1))
        & (__partitionedInclusiveProduct(productValue, mask) == productValue)
        & (__partitionedExclusiveSum(T(1), mask) == T(partitionedIndex))
        & (__partitionedExclusiveProduct(productValue, mask) == T(1))
        ;
}

__generic<T : __BuiltinArithmeticType, let N : int>
bool testVSumProduct(uint4 mask) {
    typealias GVec = vector<T, N>;

    let productValue = GVec(T(gProductValue));

    return true
        & all(__partitionedInclusiveSum(GVec(T(1)), mask) == GVec(T(partitionedIndex + 1)))
        & all(__partitionedInclusiveProduct(productValue, mask) == productValue)
        & all(__partitionedExclusiveSum(GVec(T(1)), mask) == GVec(T(partitionedIndex)))
        & all(__partitionedExclusiveProduct(productValue, mask) == GVec(T(1)))
        ;
}

bool testSumProduct(uint4 mask)
{
    return true
        & test1SumProduct<int>(mask)
        & testVSumProduct<int, 2>(mask)
        & testVSumProduct<int, 3>(mask)
        & testVSumProduct<int, 4>(mask)
        & test1SumProduct<uint>(mask)
        & testVSumProduct<uint, 2>(mask)
        & testVSumProduct<uint, 3>(mask)
        & testVSumProduct<uint, 4>(mask)
        & test1SumProduct<float>(mask)
        & testVSumProduct<float, 2>(mask)
        & testVSumProduct<float, 3>(mask)
        & testVSumProduct<float, 4>(mask)
        & test1SumProduct<double>(mask)
        & testVSumProduct<double, 2>(mask)
        & testVSumProduct<double, 3>(mask)
        & testVSumProduct<double, 4>(mask)

#if !defined(CUDA)
        & test1SumProduct<int8_t>(mask)
        & testVSumProduct<int8_t, 2>(mask)
        & testVSumProduct<int8_t, 3>(mask)
        & testVSumProduct<int8_t, 4>(mask)
        & test1SumProduct<int16_t>(mask)
        & testVSumProduct<int16_t, 2>(mask)
        & testVSumProduct<int16_t, 3>(mask)
        & testVSumProduct<int16_t, 4>(mask)
        & test1SumProduct<int64_t>(mask)
        & testVSumProduct<int64_t, 2>(mask)
        & testVSumProduct<int64_t, 3>(mask)
        & testVSumProduct<int64_t, 4>(mask)
        & test1SumProduct<uint8_t>(mask)
        & testVSumProduct<uint8_t, 2>(mask)
        & testVSumProduct<uint8_t, 3>(mask)
        & testVSumProduct<uint8_t, 4>(mask)
        & test1SumProduct<uint16_t>(mask)
        & testVSumProduct<uint16_t, 2>(mask)
        & testVSumProduct<uint16_t, 3>(mask)
        & testVSumProduct<uint16_t, 4>(mask)
        & test1SumProduct<uint64_t>(mask)
        & testVSumProduct<uint64_t, 2>(mask)
        & testVSumProduct<uint64_t, 3>(mask)
        & testVSumProduct<uint64_t, 4>(mask)
        & test1SumProduct<half>(mask)
        & testVSumProduct<half, 2>(mask)
        & testVSumProduct<half, 3>(mask)
        & testVSumProduct<half, 4>(mask)
#endif
        ;
}

[numthreads(32, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;
    partitionedIndex = index;
    bool isSecondGroup = false;

    // Split into two groups, first group has 15 invocations/lanes and second group has 17.
    uint4 mask = uint4(0x0007FFF, 0, 0, 0);
    if (index >=  15)
    {
        isSecondGroup = true;
        mask = uint4(0xFFFF8000, 0, 0, 0);
        partitionedIndex -= 15;
    }

    let isLastInPartition = (index == 14) || (index == 31);
    gProductValue = isLastInPartition ? uint(0) : uint(1);

    bool result = true
            & testSumProduct(mask)
            ;

    // CHECK-COUNT-32: 1
    outputBuffer[index] = uint(result);
}
