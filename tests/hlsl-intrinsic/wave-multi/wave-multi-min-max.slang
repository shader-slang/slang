//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-via-glsl
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute  -shaderobj -xslang -DCUDA

//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly -xslang -DUSE_GLSL_SYNTAX -allow-glsl

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

#if defined(USE_GLSL_SYNTAX)
#define __partitionedMin subgroupPartitionedMinNV
#define __partitionedMax subgroupPartitionedMaxNV
#else
#define __partitionedMin WaveMultiMin
#define __partitionedMax WaveMultiMax
#endif


static uint gMinResult = 0;
static uint gMaxResult = 0;
static uint gMinMaxValue = 0;

__generic<T : __BuiltinArithmeticType>
bool test1MinMax(uint4 mask)
{
    let minResult = T(gMinResult);
    let maxResult = T(gMaxResult);
    let minMaxValue = T(gMinMaxValue);

    return true
        & all(__partitionedMin(minMaxValue, mask) == minResult)
        & all(__partitionedMax(minMaxValue, mask) == maxResult)
        ;
}

__generic<T : __BuiltinArithmeticType, let N : int>
bool testVMinMax(uint4 mask) {
    typealias GVec = vector<T, N>;

    let minResult = GVec(T(gMinResult));
    let maxResult = GVec(T(gMaxResult));
    let minMaxValue = GVec(T(gMinMaxValue));

    return true
        & all(__partitionedMin(minMaxValue, mask) == minResult)
        & all(__partitionedMax(minMaxValue, mask) == maxResult)
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

#if !defined(CUDA)
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
#endif
        ;
}

[numthreads(32, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint index = dispatchThreadID.x;

    // Split into two groups, first group has 15 invocations/lanes and second group has 17.
    let isSecondGroup = index >= 15;
    uint4 mask = isSecondGroup ? uint4(0xFFFF8000, 0, 0, 0) : uint4(0x0007FFF, 0, 0, 0);

    // Set min value on one invocation on each partition/mask.
    let isMinInvocation = (index == 0) || (index == 15);

    gMinResult = isSecondGroup ? uint(2) : uint(0);
    gMaxResult = isSecondGroup ? uint(3) : uint(1);
    gMinMaxValue = isMinInvocation ? gMinResult : gMaxResult;

    bool result = true
            && testMinMax(mask)
            ;

    // CHECK-COUNT-32: 1
    outputBuffer[index] = uint(result);
}
