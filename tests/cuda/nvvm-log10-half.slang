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

//TEST_INPUT: ubuffer(data=[0 1 64512 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51002 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 50924 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 50231 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 50231 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 50231 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49577 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49577 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49577 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47930 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47930 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47929 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47364 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 47363 0 47364 0 0 0 0 0 0 0 0 0 0 0 0 1 47363 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47317 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 47316 0 47317 0 0 0 0 0 0 0 0 0 0 0 0 1 47316 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47313 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47313 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47312 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47202 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47201 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47201 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 46290 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 46289 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 46287 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 45058 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 45055 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 45050 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 40029 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 39973 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 39898 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 37175 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 36596 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 35571 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 3826 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 7250 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 7360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 7470 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 12704 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 12707 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 12709 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13520 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13521 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13523 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14241 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14242 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14243 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14545 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14545 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14546 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15161 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15162 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15162 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15359 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16384 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16384 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16384 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16435 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 16435 0 16436 0 0 0 0 0 0 0 0 0 0 0 0 1 16436 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16764 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16764 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16764 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16896 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16896 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16896 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17423 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 17423 0 17424 0 0 0 0 0 0 0 0 0 0 0 0 1 17424 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17617 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 64512 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[0 1 64512 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51002 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 50924 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 50231 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 50231 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 50231 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49577 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49577 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49577 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47930 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47930 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47929 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47364 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47363 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47363 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47317 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47317 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47316 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47313 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47313 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47312 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47202 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47201 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47201 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 46290 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 46289 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 46287 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 45058 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 45055 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 45050 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 40029 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 39973 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 39898 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 37175 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 36596 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 35571 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 3826 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 7250 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 7360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 7470 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 12704 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 12707 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 12709 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13520 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13521 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13523 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14241 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14242 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14243 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14545 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14545 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14546 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15161 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15162 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15162 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15359 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16384 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16384 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16384 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16435 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16435 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16436 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16764 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16764 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16764 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16896 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16896 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16896 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17423 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17423 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17424 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17617 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 64512 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):name=cudaWords
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
half log10ThroughHelper(half value)
{
    return log10(value);
}

[numthreads(128, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 91) return;
    uint errors = 0;
    half scalarResult = log10(loadValue(lane));
    errors |= matchesExpected(scalarResult, lane) ? 0u : 1u;
    errors |= matchesExpected(log10ThroughHelper(loadValue(lane)), lane) ? 0u : 2u;
    half2 result2 = log10(half2(loadValue((lane + 0) % 91), loadValue((lane + 1) % 91)));
    errors |= matchesExpected(result2.x, (lane + 0) % 91) ? 0u : 4u;
    errors |= matchesExpected(result2.y, (lane + 1) % 91) ? 0u : 4u;
    half3 result3 = log10(half3(loadValue((lane + 0) % 91), loadValue((lane + 1) % 91), loadValue((lane + 2) % 91)));
    errors |= matchesExpected(result3.x, (lane + 0) % 91) ? 0u : 8u;
    errors |= matchesExpected(result3.y, (lane + 1) % 91) ? 0u : 8u;
    errors |= matchesExpected(result3.z, (lane + 2) % 91) ? 0u : 8u;
    half4 result4 = log10(half4(loadValue((lane + 0) % 91), loadValue((lane + 1) % 91), loadValue((lane + 2) % 91), loadValue((lane + 3) % 91)));
    errors |= matchesExpected(result4.x, (lane + 0) % 91) ? 0u : 16u;
    errors |= matchesExpected(result4.y, (lane + 1) % 91) ? 0u : 16u;
    errors |= matchesExpected(result4.z, (lane + 2) % 91) ? 0u : 16u;
    errors |= matchesExpected(result4.w, (lane + 3) % 91) ? 0u : 16u;
    matrix<half, 2, 2> resultMatrix = log10(matrix<half, 2, 2>(loadValue((lane + 0) % 91), loadValue((lane + 1) % 91), loadValue((lane + 2) % 91), loadValue((lane + 3) % 91)));
    errors |= matchesExpected(resultMatrix[0][0], (lane + 0) % 91) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[0][1], (lane + 1) % 91) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][0], (lane + 2) % 91) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][1], (lane + 3) % 91) ? 0u : 32u;
    uint low, high;
    valueBits(scalarResult, low, high);
    if (lane == 0) outputBuffer[1] = usesLibraryPolicy() ? 62043u : 1209u;
    outputBuffer[2 + 4 * lane] = errors;
    outputBuffer[3 + 4 * lane] = low;
    outputBuffer[4 + 4 * lane] = high;
    outputBuffer[5 + 4 * lane] = 62000 + lane;
}

// CUDA: 324508639{{$}}
// CUDA-NEXT: 1209{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62000{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62001{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62002{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62003{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62004{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62005{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62006{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62007{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62008{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62009{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62010{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62011{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62012{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62013{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62014{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62015{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62016{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62017{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62018{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62019{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62020{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62021{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62022{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62023{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62024{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62025{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62026{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62027{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62028{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62029{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62030{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62031{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62032{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62033{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62034{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62035{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62036{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62037{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62038{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62039{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62040{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62041{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62042{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62043{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62044{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62045{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62046{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62047{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62048{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62049{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62050{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62051{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62052{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62053{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62054{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62055{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62056{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62057{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62058{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62059{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62060{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62061{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62062{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62063{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62064{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62065{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62066{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62067{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62068{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62069{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62070{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62071{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62072{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62073{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62074{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62075{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62076{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62077{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62078{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62079{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62080{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62081{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62082{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62083{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62084{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62085{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62086{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62087{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62088{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62089{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 62090{{$}}
// CUDA-NEXT: 3735928559{{$}}
// NVVM: 324508639{{$}}
// NVVM-NEXT: 62043{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62000{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62001{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62002{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62003{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62004{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62005{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62006{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62007{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62008{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62009{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62010{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62011{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62012{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62013{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62014{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62015{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62016{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62017{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62018{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62019{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62020{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62021{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62022{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62023{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62024{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62025{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62026{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62027{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62028{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62029{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62030{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62031{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62032{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62033{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62034{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62035{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62036{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62037{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62038{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62039{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62040{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62041{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62042{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62043{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62044{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62045{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62046{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62047{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62048{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62049{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62050{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62051{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62052{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62053{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62054{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62055{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62056{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62057{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62058{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62059{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62060{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62061{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62062{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62063{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62064{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62065{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62066{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62067{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62068{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62069{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62070{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62071{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62072{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62073{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62074{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62075{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62076{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62077{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62078{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62079{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62080{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62081{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62082{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62083{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62084{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62085{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62086{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62087{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62088{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62089{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 62090{{$}}
// NVVM-NEXT: 3735928559{{$}}
