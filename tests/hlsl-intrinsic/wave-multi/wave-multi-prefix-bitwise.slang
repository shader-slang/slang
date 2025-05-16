//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -xslang DVK
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-slang -compute -dx12 -use-dxil -profile sm_6_5 -shaderobj
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute  -shaderobj -xslang -DCUDA

//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly -xslang -DUSE_GLSL_SYNTAX -allow-glsl

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

#if defined(USE_GLSL_SYNTAX)
#define __partitionedInclusiveAnd subgroupPartitionedInclusiveAndNV
#define __partitionedInclusiveOr subgroupPartitionedInclusiveOrNV
#define __partitionedInclusiveXor subgroupPartitionedInclusiveXorNV
#define __partitionedExclusiveAnd subgroupPartitionedExclusiveAndNV
#define __partitionedExclusiveOr subgroupPartitionedExclusiveOrNV
#define __partitionedExclusiveXor subgroupPartitionedExclusiveXorNV
#else
#define __partitionedInclusiveAnd WaveMultiPrefixInclusiveBitAnd
#define __partitionedInclusiveOr WaveMultiPrefixInclusiveBitOr
#define __partitionedInclusiveXor WaveMultiPrefixInclusiveBitXor
#define __partitionedExclusiveAnd WaveMultiPrefixExclusiveBitAnd
#define __partitionedExclusiveOr WaveMultiPrefixExclusiveBitOr
#define __partitionedExclusiveXor WaveMultiPrefixExclusiveBitXor
#endif


static uint gAndValue = 0;
static uint gAndResultExclusive = 0;
static uint gOrValue = 0;
static uint gOrResult = 0;
static uint gXorValue = 0;
static uint gXorResultInclusive = 0;
static uint gXorResultExclusive = 0;

__generic<T : __BuiltinLogicalType>
bool test1Bitwise(uint4 mask)
{
    let andValue = T(gAndValue);
    let orValue = T(gOrValue);
    let xorValue = T(gXorValue);

    return true
        & (__partitionedInclusiveAnd(andValue, mask) == andValue)
        & (__partitionedExclusiveAnd(andValue, mask) == T(gAndResultExclusive))
        & (__partitionedInclusiveOr(orValue, mask) == orValue)
        & (__partitionedExclusiveOr(orValue, mask) == T(0))
        & (__partitionedInclusiveXor(xorValue, mask) == T(gXorResultInclusive))
        & (__partitionedExclusiveXor(xorValue, mask) == T(gXorResultExclusive))
        ;
}

__generic<T : __BuiltinLogicalType, let N : int>
bool testVBitwise(uint4 mask) {
    typealias GVec = vector<T, N>;

    let andValue = GVec(T(gAndValue));
    let orValue = GVec(T(gOrValue));
    let xorValue = GVec(T(gXorValue));

    return true
        & all(__partitionedInclusiveAnd(andValue, mask) == andValue)
        & all(__partitionedExclusiveAnd(andValue, mask) == GVec(T(gAndResultExclusive)))
        & all(__partitionedInclusiveOr(orValue, mask) == orValue)
        & all(__partitionedExclusiveOr(orValue, mask) == GVec(T(0)))
        & all(__partitionedInclusiveXor(xorValue, mask) == GVec(T(gXorResultInclusive)))
        & all(__partitionedExclusiveXor(xorValue, mask) == GVec(T(gXorResultExclusive)))
        ;
}

bool testBitwise(uint4 mask)
{
    return true
        & test1Bitwise<int>(mask)
        & testVBitwise<int, 2>(mask)
        & testVBitwise<int, 3>(mask)
        & testVBitwise<int, 4>(mask)
        & test1Bitwise<uint>(mask)
        & testVBitwise<uint, 2>(mask)
        & testVBitwise<uint, 3>(mask)
        & testVBitwise<uint, 4>(mask)

        // TODO: these are failing SPIRV validation and should be fixed.
        // SPIRV's ops do not directly accept/return bool.
        // & test1Bitwise<bool>(mask)
        // & testVBitwise<bool, 2>(mask)
        // & testVBitwise<bool, 3>(mask)
        // & testVBitwise<bool, 4>(mask)

#if defined(VK)
        & test1Bitwise<int8_t>(mask)
        & testVBitwise<int8_t, 2>(mask)
        & testVBitwise<int8_t, 3>(mask)
        & testVBitwise<int8_t, 4>(mask)
        & test1Bitwise<uint8_t>(mask)
        & testVBitwise<uint8_t, 2>(mask)
        & testVBitwise<uint8_t, 3>(mask)
        & testVBitwise<uint8_t, 4>(mask)
#endif

#if !defined(CUDA)
        & test1Bitwise<int16_t>(mask)
        & testVBitwise<int16_t, 2>(mask)
        & testVBitwise<int16_t, 3>(mask)
        & testVBitwise<int16_t, 4>(mask)
        & test1Bitwise<int64_t>(mask)
        & testVBitwise<int64_t, 2>(mask)
        & testVBitwise<int64_t, 3>(mask)
        & testVBitwise<int64_t, 4>(mask)
        & test1Bitwise<uint16_t>(mask)
        & testVBitwise<uint16_t, 2>(mask)
        & testVBitwise<uint16_t, 3>(mask)
        & testVBitwise<uint16_t, 4>(mask)
        & test1Bitwise<uint64_t>(mask)
        & testVBitwise<uint64_t, 2>(mask)
        & testVBitwise<uint64_t, 3>(mask)
        & testVBitwise<uint64_t, 4>(mask)
#endif
        ;
}

[numthreads(32, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    let index = dispatchThreadID.x;

    let isSecondGroup = index >= 15;
    let mask = isSecondGroup ? uint4(0xFFFF8000, 0, 0, 0) : uint4(0x0007FFF, 0, 0, 0);
    let isLastInvocation = (index == 31);
    let isLastInPartition = (index == 14) || (index == 31);
    let isFirstInPartition = (index == 0) || (index == 15);

    //
    // Prefix and.
    // - Both groups use 1 except for the last invocation in each partition where input is 0.
    // - For inclusive ops, result is 1 except for last invocation in each partition.
    // - For exclusive ops, first in partition is always results to ~0(identity). Otherwise exclusive ops result to 1.
    gAndValue = isLastInPartition ? uint(0) : uint(1);
    gAndResultExclusive = isFirstInPartition ? uint(~0) : uint(1);

    //
    // Prefix or.
    // - Both groups use 0 except for the last invocation in each partition where input is 1.
    // - For inclusive ops, result is 0 except for last invocation in each partition.
    // - For exclusive ops, result is always 0.
    gOrValue = isLastInPartition ? uint(1) : uint(0);

    // Prefix xor.
    // - First group input is always 1. Inclusive results alternate between 1 and 0, starting at 1. Exclusive result is also alternates but starts at 0 (opposite of inclusive result).
    // - Second group is always 0. Results are all 0.
    gXorValue = isSecondGroup ? uint(0) : uint(1);
    gXorResultInclusive = (isSecondGroup || (index % 2 != 0)) ? uint(0) : uint(1);
    gXorResultExclusive = isSecondGroup ? uint(0) : (uint(1) - gXorResultInclusive);

    bool result = true
            & testBitwise(mask)
            ;

    // CHECK-COUNT-32: 1
    outputBuffer[index] = uint(result);
}
