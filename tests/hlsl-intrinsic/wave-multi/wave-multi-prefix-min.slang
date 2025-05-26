//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-via-glsl

//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly -xslang -DUSE_GLSL_SYNTAX -allow-glsl

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

#if defined(USE_GLSL_SYNTAX)
#define __partitionedInclusiveMin subgroupPartitionedInclusiveMinNV
#define __partitionedExclusiveMin subgroupPartitionedExclusiveMinNV
#else
#define __partitionedInclusiveMin WaveMultiPrefixInclusiveMin
#define __partitionedExclusiveMin WaveMultiPrefixExclusiveMin
#endif

static bool isFirstInPartition = false;
static uint gSmaller = 0;
static uint gLarger = 0;
static uint gMaxValue = 0;

__generic<T : __BuiltinArithmeticType>
bool test1Min(uint4 mask)
{
    let larger = T(gLarger);
    let minValue = T(gMaxValue);

    // The smaller values are set to be the last in the partition, exclusive variants will never get these values.
    bool exclusiveRes = true
                        & (__partitionedExclusiveMin(minValue, mask) == larger)
                        ;
    // Do not check exclusive prefix for the first invocation in partition as their values(identity values) depend on the builtin type `T`. It would be
    // nice to have something like T::min or T::max.
    if (isFirstInPartition)
    {
        exclusiveRes = true;
    }

    return true
        & (__partitionedInclusiveMin(minValue, mask) == minValue)
        & exclusiveRes
        ;
}

__generic<T : __BuiltinArithmeticType, let N : int>
bool testVMin(uint4 mask) {
    typealias GVec = vector<T, N>;

    let larger = GVec(T(gLarger));
    let minValue = GVec(T(gMaxValue));

    // The smaller values are set to be the last in the partition, exclusive variants will never get these values.
    bool exclusiveRes = true
                        & all(__partitionedExclusiveMin(minValue, mask) == larger)
                        ;
    // Do not check exclusive prefix for the first invocation in partition as their values(identity values) depend on the builtin type `T`. It would be
    // nice to have something like T::min or T::max.
    if (isFirstInPartition)
    {
        exclusiveRes = true;
    }

    return true
        & all(__partitionedInclusiveMin(minValue, mask) == minValue)
        & exclusiveRes
        ;
}

bool testMin(uint4 mask)
{
    return true
        & test1Min<int>(mask)
        & testVMin<int, 2>(mask)
        & testVMin<int, 3>(mask)
        & testVMin<int, 4>(mask)
        & test1Min<uint>(mask)
        & testVMin<uint, 2>(mask)
        & testVMin<uint, 3>(mask)
        & testVMin<uint, 4>(mask)
        & test1Min<float>(mask)
        & testVMin<float, 2>(mask)
        & testVMin<float, 3>(mask)
        & testVMin<float, 4>(mask)
        & test1Min<double>(mask)
        & testVMin<double, 2>(mask)
        & testVMin<double, 3>(mask)
        & testVMin<double, 4>(mask)
        & test1Min<int8_t>(mask)
        & testVMin<int8_t, 2>(mask)
        & testVMin<int8_t, 3>(mask)
        & testVMin<int8_t, 4>(mask)
        & test1Min<int16_t>(mask)
        & testVMin<int16_t, 2>(mask)
        & testVMin<int16_t, 3>(mask)
        & testVMin<int16_t, 4>(mask)
        & test1Min<int64_t>(mask)
        & testVMin<int64_t, 2>(mask)
        & testVMin<int64_t, 3>(mask)
        & testVMin<int64_t, 4>(mask)
        & test1Min<uint8_t>(mask)
        & testVMin<uint8_t, 2>(mask)
        & testVMin<uint8_t, 3>(mask)
        & testVMin<uint8_t, 4>(mask)
        & test1Min<uint16_t>(mask)
        & testVMin<uint16_t, 2>(mask)
        & testVMin<uint16_t, 3>(mask)
        & testVMin<uint16_t, 4>(mask)
        & test1Min<uint64_t>(mask)
        & testVMin<uint64_t, 2>(mask)
        & testVMin<uint64_t, 3>(mask)
        & testVMin<uint64_t, 4>(mask)
        & test1Min<half>(mask)
        & testVMin<half, 2>(mask)
        & testVMin<half, 3>(mask)
        & testVMin<half, 4>(mask)
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

    bool result = true
            & testMin(mask)
            ;

    gSmaller = isSecondGroup ? 2 : 0;
    gLarger = isSecondGroup ? 3 : 1;
    gMaxValue = isLastInPartition ? gLarger : gSmaller;

    // CHECK-COUNT-32: 1
    outputBuffer[index] = uint(result);
}
