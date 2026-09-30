// Independent signed logarithm references are frozen before GPU observation.
// Library admission uses the empirical test-defined signed spacing/encoding union.
// NVVM Half narrows Float32 candidates once; CUDA Half checks documented RN-even
// on this finite corpus. CUDA double log10 preserves its existing Float32 wrapper.
// Every mode must also preserve its own complete baseline buffer byte for byte.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CUDA): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=NVVM): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=NVVM): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT: ubuffer(data=[0 0 1 0 2 0 1023 0 1024 0 1025 0 5644 0 5645 0 5646 0 12287 0 12288 0 12289 0 13198 0 13199 0 13200 0 13303 0 13304 0 13305 0 13311 0 13312 0 13313 0 13448 0 13449 0 13450 0 14335 0 14336 0 14337 0 14847 0 14848 0 14849 0 15340 0 15341 0 15342 0 15357 0 15358 0 15359 0 15360 0 15361 0 15370 0 15371 0 15372 0 15871 0 15872 0 15873 0 16383 0 16384 0 16385 0 16895 0 16896 0 16897 0 17407 0 17408 0 17409 0 18431 0 18432 0 18433 0 18687 0 18688 0 18689 0 22079 0 22080 0 22081 0 22496 0 22497 0 22498 0 24656 0 24657 0 24658 0 25551 0 25552 0 25553 0 29084 0 29085 0 29086 0 31743 0 31744 0 31745 0 32256 0 32768 0 47103 0 47104 0 47105 0 48127 0 48128 0 48129 0 49151 0 49152 0 49153 0 64512 0 64513 0 65024 0], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 1 64512 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 52736 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 52672 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51968 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51968 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51968 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51380 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51380 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51379 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49664 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49664 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49663 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49194 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49194 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49194 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49155 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49155 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49155 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49152 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49152 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49151 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48968 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 48966 0 48967 0 0 0 0 0 0 0 0 0 0 0 0 1 48965 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48129 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48128 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48125 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 46760 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 46756 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 46752 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 41792 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 41699 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 41605 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 38997 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 38342 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 37318 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 5573 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 9006 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 9188 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 9294 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14508 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14510 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14512 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15359 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15361 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15958 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15959 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15960 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16383 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16384 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16385 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16896 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16896 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16897 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17060 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17061 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17061 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18085 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18085 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18085 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18170 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18170 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18171 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18574 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18574 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18574 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18684 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18684 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18684 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 19134 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 19135 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 19135 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 19456 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 64512 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[0 1 64512 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 52736 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 52672 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51968 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51968 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51968 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51380 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51380 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51379 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49664 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49664 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49663 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49194 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49194 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49194 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49155 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49155 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49155 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49152 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49152 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49151 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48968 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48967 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48965 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48129 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48128 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48125 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 46760 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 46756 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 46752 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 41792 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 41699 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 41605 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 38997 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 38342 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 37318 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 5573 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 9006 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 9188 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 9294 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14508 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14510 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14512 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15359 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15361 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15958 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15959 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15960 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16383 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16384 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16385 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16896 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16896 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16897 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17060 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17061 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17061 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18085 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18085 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18085 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18170 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18170 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18171 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18574 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18574 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18574 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18684 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18684 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18684 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 19134 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 19135 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 19135 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 19456 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 64512 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):name=cudaWords
StructuredBuffer<uint> cudaWords;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

half loadValue(uint lane)
{
    return bit_cast<half>(uint16_t(inputWords[2 * lane]));
}

void valueBits(half value, out uint low, out uint high)
{
    low = uint(bit_cast<uint16_t>(value));
    high = 0;
}

bool usesLibraryPolicy()
{
    __target_switch
    {
    case nvvm: return true;
    default: return false;
    }
}

uint expectedWord(uint offset)
{
    return usesLibraryPolicy() ? expectedWords[offset] : cudaWords[offset];
}

bool matchesExpected(half value, uint lane)
{
    uint low, high;
    valueBits(value, low, high);
    bool nanBits = (low & 0x7c00u) == 0x7c00u && (low & 0x3ffu) != 0;
    uint offset = 16 * lane;
    if (expectedWord(offset) != 0) return nanBits;
    uint count = expectedWord(offset + 1);
    for (uint choice = 0; choice < count; ++choice)
    {
        if (low == expectedWord(offset + 2 + 2 * choice) &&
            high == expectedWord(offset + 3 + 2 * choice)) return true;
    }
    return false;
}

[noinline]
half log2ThroughHelper(half value)
{
    return log2(value);
}

[numthreads(128, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 91) return;
    uint errors = 0;
    half scalarResult = log2(loadValue(lane));
    errors |= matchesExpected(scalarResult, lane) ? 0u : 1u;
    errors |= matchesExpected(log2ThroughHelper(loadValue(lane)), lane) ? 0u : 2u;
    half2 result2 = log2(half2(loadValue((lane + 0) % 91), loadValue((lane + 1) % 91)));
    errors |= matchesExpected(result2.x, (lane + 0) % 91) ? 0u : 4u;
    errors |= matchesExpected(result2.y, (lane + 1) % 91) ? 0u : 4u;
    half3 result3 = log2(half3(loadValue((lane + 0) % 91), loadValue((lane + 1) % 91), loadValue((lane + 2) % 91)));
    errors |= matchesExpected(result3.x, (lane + 0) % 91) ? 0u : 8u;
    errors |= matchesExpected(result3.y, (lane + 1) % 91) ? 0u : 8u;
    errors |= matchesExpected(result3.z, (lane + 2) % 91) ? 0u : 8u;
    half4 result4 = log2(half4(loadValue((lane + 0) % 91), loadValue((lane + 1) % 91), loadValue((lane + 2) % 91), loadValue((lane + 3) % 91)));
    errors |= matchesExpected(result4.x, (lane + 0) % 91) ? 0u : 16u;
    errors |= matchesExpected(result4.y, (lane + 1) % 91) ? 0u : 16u;
    errors |= matchesExpected(result4.z, (lane + 2) % 91) ? 0u : 16u;
    errors |= matchesExpected(result4.w, (lane + 3) % 91) ? 0u : 16u;
    matrix<half, 2, 2> resultMatrix = log2(matrix<half, 2, 2>(loadValue((lane + 0) % 91), loadValue((lane + 1) % 91), loadValue((lane + 2) % 91), loadValue((lane + 3) % 91)));
    errors |= matchesExpected(resultMatrix[0][0], (lane + 0) % 91) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[0][1], (lane + 1) % 91) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][0], (lane + 2) % 91) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][1], (lane + 3) % 91) ? 0u : 32u;
    uint low, high;
    valueBits(scalarResult, low, high);
    if (lane == 0) outputBuffer[1] = usesLibraryPolicy() ? 61043u : 1209u;
    outputBuffer[2 + 4 * lane] = errors;
    outputBuffer[3 + 4 * lane] = low;
    outputBuffer[4 + 4 * lane] = high;
    outputBuffer[5 + 4 * lane] = 61000 + lane;
}

// CUDA: 324508639{{$}}
// CUDA-NEXT: 1209{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61000{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61001{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61002{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61003{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61004{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61005{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61006{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61007{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61008{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61009{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61010{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61011{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61012{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61013{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61014{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61015{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61016{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61017{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61018{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61019{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61020{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61021{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61022{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61023{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61024{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61025{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61026{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61027{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61028{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61029{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61030{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61031{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61032{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61033{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61034{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61035{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61036{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61037{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61038{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61039{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61040{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61041{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61042{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61043{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61044{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61045{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61046{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61047{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61048{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61049{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61050{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61051{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61052{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61053{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61054{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61055{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61056{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61057{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61058{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61059{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61060{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61061{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61062{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61063{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61064{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61065{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61066{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61067{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61068{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61069{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61070{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61071{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61072{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61073{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61074{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61075{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61076{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61077{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61078{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61079{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61080{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61081{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61082{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61083{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61084{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61085{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61086{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61087{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61088{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61089{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 61090{{$}}
// CUDA-NEXT: 3735928559{{$}}
// NVVM: 324508639{{$}}
// NVVM-NEXT: 61043{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61000{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61001{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61002{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61003{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61004{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61005{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61006{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61007{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61008{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61009{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61010{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61011{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61012{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61013{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61014{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61015{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61016{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61017{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61018{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61019{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61020{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61021{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61022{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61023{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61024{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61025{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61026{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61027{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61028{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61029{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61030{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61031{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61032{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61033{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61034{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61035{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61036{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61037{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61038{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61039{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61040{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61041{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61042{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61043{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61044{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61045{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61046{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61047{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61048{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61049{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61050{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61051{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61052{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61053{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61054{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61055{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61056{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61057{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61058{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61059{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61060{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61061{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61062{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61063{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61064{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61065{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61066{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61067{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61068{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61069{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61070{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61071{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61072{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61073{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61074{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61075{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61076{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61077{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61078{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61079{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61080{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61081{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61082{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61083{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61084{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61085{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61086{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61087{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61088{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61089{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 61090{{$}}
// NVVM-NEXT: 3735928559{{$}}
