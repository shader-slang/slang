//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-via-glsl
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -capability cuda_sm_7_0 -compute -shaderobj
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly -xslang -DUSE_GLSL_SYNTAX -allow-glsl

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

#if defined(USE_GLSL_SYNTAX)
#define __partitionedInclusiveMax subgroupPartitionedInclusiveMaxNV
#define __partitionedExclusiveMax subgroupPartitionedExclusiveMaxNV
#else
#define __partitionedInclusiveMax WaveMultiPrefixInclusiveMax
#define __partitionedExclusiveMax WaveMultiPrefixExclusiveMax
#endif

static bool isFirstInPartition = false;
static uint gSmaller = 0;
static uint gLarger = 0;
static uint gMaxValue = 0;

__generic<T : __BuiltinArithmeticType>
bool test1MinMax(uint4 mask)
{
    let smaller = T(gSmaller);
    let maxValue = T(gMaxValue);

    // The larger values are set to be the last in the partition, exclusive variants will never get these values.
    bool exclusiveRes = true
                        & (__partitionedExclusiveMax(maxValue, mask) == smaller)
                        ;
    // Do not check exclusive prefix for the first invocation in partition as their values(identity values) depend on the builtin type `T`. It would be
    // nice to have something like T::min or T::max.
    if (isFirstInPartition)
    {
        exclusiveRes = true;
    }

    return true
        & (__partitionedInclusiveMax(maxValue, mask) == maxValue)
        & exclusiveRes
        ;
}

__generic<T : __BuiltinArithmeticType, let N : int>
bool testVMinMax(uint4 mask) {
    typealias GVec = vector<T, N>;

    let smaller = GVec(T(gSmaller));
    let maxValue = GVec(T(gMaxValue));

    // The larger values are set to be the last in the partition, exclusive variants will never get these values.
    bool exclusiveRes = true
                        & all(__partitionedExclusiveMax(maxValue, mask) == smaller)
                        ;
    // Do not check exclusive prefix for the first invocation in partition as their values(identity values) depend on the builtin type `T`. It would be
    // nice to have something like T::min or T::max.
    if (isFirstInPartition)
    {
        exclusiveRes = true;
    }

    return true
        & all(__partitionedInclusiveMax(maxValue, mask) == maxValue)
        & exclusiveRes;
        ;
}

bool testMinMax(uint4 mask)
{
    return true
        & test1MinMax<int>(mask)
        & testVMinMax<int, 2>(mask)
        & testVMinMax<int, 3>(mask)
        & testVMinMax<int, 4>(mask)
        & test1MinMax<uint>(mask)
        & testVMinMax<uint, 2>(mask)
        & testVMinMax<uint, 3>(mask)
        & testVMinMax<uint, 4>(mask)
        & test1MinMax<float>(mask)
        & testVMinMax<float, 2>(mask)
        & testVMinMax<float, 3>(mask)
        & testVMinMax<float, 4>(mask)
        & test1MinMax<double>(mask)
        & testVMinMax<double, 2>(mask)
        & testVMinMax<double, 3>(mask)
        & testVMinMax<double, 4>(mask)
        & test1MinMax<int8_t>(mask)
        & testVMinMax<int8_t, 2>(mask)
        & testVMinMax<int8_t, 3>(mask)
        & testVMinMax<int8_t, 4>(mask)
        & test1MinMax<int16_t>(mask)
        & testVMinMax<int16_t, 2>(mask)
        & testVMinMax<int16_t, 3>(mask)
        & testVMinMax<int16_t, 4>(mask)
        & test1MinMax<int64_t>(mask)
        & testVMinMax<int64_t, 2>(mask)
        & testVMinMax<int64_t, 3>(mask)
        & testVMinMax<int64_t, 4>(mask)
        & test1MinMax<uint8_t>(mask)
        & testVMinMax<uint8_t, 2>(mask)
        & testVMinMax<uint8_t, 3>(mask)
        & testVMinMax<uint8_t, 4>(mask)
        & test1MinMax<uint16_t>(mask)
        & testVMinMax<uint16_t, 2>(mask)
        & testVMinMax<uint16_t, 3>(mask)
        & testVMinMax<uint16_t, 4>(mask)
        & test1MinMax<uint64_t>(mask)
        & testVMinMax<uint64_t, 2>(mask)
        & testVMinMax<uint64_t, 3>(mask)
        & testVMinMax<uint64_t, 4>(mask)
        & test1MinMax<half>(mask)
        & testVMinMax<half, 2>(mask)
        & testVMinMax<half, 3>(mask)
        & testVMinMax<half, 4>(mask)
        ;
}

[numthreads(32, 1, 1)]
[shader("compute")]
[MaximallyReconverges]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    let index = dispatchThreadID.x;

    // Split into two groups, first group has 15 invocations/lanes and second group has 17.
    let isSecondGroup = index >= 15;
    uint4 mask = isSecondGroup ? uint4(0xFFFF8000, 0, 0, 0) : uint4(0x0007FFF, 0, 0, 0);

    isFirstInPartition = (index == 0) || (index == 15);
    let isLastInPartition = (index == 14) || (index == 31);

    gSmaller = isSecondGroup ? 2 : 0;
    gLarger = isSecondGroup ? 3 : 1;
    gMaxValue = isLastInPartition ? gLarger : gSmaller;

    bool result = true
            & testMinMax(mask)
            ;

    // CHECK-COUNT-32: 1
    outputBuffer[index] = uint(result);
}
