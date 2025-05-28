//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-via-glsl
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -compute  -shaderobj -xslang -DCUDA

//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-vk -compute -shaderobj -emit-spirv-directly -xslang -DUSE_GLSL_SYNTAX -allow-glsl

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

#if defined(USE_GLSL_SYNTAX)
#define __partitionedAnd subgroupPartitionedAndNV
#define __partitionedOr subgroupPartitionedOrNV
#define __partitionedXor subgroupPartitionedXorNV
#else
#define __partitionedAnd WaveMultiBitAnd
#define __partitionedOr WaveMultiBitOr
#define __partitionedXor WaveMultiBitXor
#endif

static uint gAndValue = 0;
static uint gOrValue = 0;
static uint gOrResult = 0;
static uint gXorValue = 0;
static uint gXorResult = 0;

__generic<T : __BuiltinLogicalType>
bool test1Bitwise(uint4 mask)
{
    let andValue = T(gAndValue);
    let orValue = T(gOrValue);
    let orResult = T(gOrResult);
    let xorValue = T(gXorValue);
    let xorResult = T(gXorResult);

    return true
        & (__partitionedAnd(andValue, mask) == andValue)
        & (__partitionedOr(orValue, mask) == orResult)
        & (__partitionedXor(xorValue, mask) == xorResult)
        ;
}

__generic<T : __BuiltinLogicalType, let N : int>
bool testVBitwise(uint4 mask) {
    typealias GVec = vector<T, N>;

    let andValue = GVec(T(gAndValue));
    let orValue = GVec(T(gOrValue));
    let orResult = GVec(T(gOrResult));
    let xorValue = GVec(T(gXorValue));
    let xorResult = GVec(T(gXorResult));

    return true
        & all(__partitionedAnd(andValue, mask) == andValue)
        & all(__partitionedOr(orValue, mask) == orResult)
        & all(__partitionedXor(xorValue, mask) == xorResult)
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

#if !defined(CUDA)
        & test1Bitwise<int8_t>(mask)
        & testVBitwise<int8_t, 2>(mask)
        & testVBitwise<int8_t, 3>(mask)
        & testVBitwise<int8_t, 4>(mask)
        & test1Bitwise<int16_t>(mask)
        & testVBitwise<int16_t, 2>(mask)
        & testVBitwise<int16_t, 3>(mask)
        & testVBitwise<int16_t, 4>(mask)
        & test1Bitwise<int64_t>(mask)
        & testVBitwise<int64_t, 2>(mask)
        & testVBitwise<int64_t, 3>(mask)
        & testVBitwise<int64_t, 4>(mask)
        & test1Bitwise<uint8_t>(mask)
        & testVBitwise<uint8_t, 2>(mask)
        & testVBitwise<uint8_t, 3>(mask)
        & testVBitwise<uint8_t, 4>(mask)
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

    // One invocation in second group is different from others to test or and xor operations.
    let isOrSet = (index == 15);

    gAndValue = isSecondGroup ? uint(1) : uint(0);
    gOrValue = isOrSet ? uint(1) : uint(0);
    gOrResult = isSecondGroup ? uint(1) : uint(0);

    // Alternate 0s and 1s for xor.
    gXorValue = (index % 2 == 0) ? uint(0) : uint(1);
    if (isOrSet)
    {
        // This is in second group - disrupt the alternating sequence.
        gXorValue = uint(0);
    }
    gXorResult = isSecondGroup ? uint(0) : uint(1);

    bool result = true
            & testBitwise(mask)
            ;

    // CHECK-COUNT-32: 1
    outputBuffer[index] = uint(result);
}
