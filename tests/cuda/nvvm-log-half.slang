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

//TEST_INPUT: ubuffer(data=[0 1 64512 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 52265 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 52217 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51418 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51418 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51418 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 50821 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 50820 0 50821 0 0 0 0 0 0 0 0 0 0 0 0 1 50820 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49193 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49193 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49192 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48582 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48582 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48581 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48528 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48528 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48527 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48524 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48524 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48523 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48396 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48395 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48394 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47501 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47500 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47498 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 46237 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 46234 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 46232 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 41222 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 41158 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 41093 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 38401 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 37889 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 36864 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 5119 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 8442 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 8568 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 8695 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13946 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13949 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13951 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14731 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14732 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14734 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15460 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15461 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15462 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15755 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15756 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15757 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16424 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16425 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16425 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16539 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16539 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16539 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17563 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17563 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17563 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17622 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17622 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17622 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 18000 0 18001 0 0 0 0 0 0 0 0 0 0 0 0 1 18001 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18152 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18152 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18153 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18605 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18605 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18605 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18828 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 64512 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[0 1 64512 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 52265 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 52217 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51418 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51418 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 51418 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 50821 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 50821 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 50820 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49193 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49193 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 49192 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48582 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48582 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48581 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48528 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48528 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48527 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48524 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48524 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48523 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48396 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48395 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 48394 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47501 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47500 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 47498 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 46237 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 46234 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 46232 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 41222 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 41158 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 41093 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 38401 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 37889 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 36864 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 5119 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 8442 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 8568 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 8695 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13946 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13949 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13951 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14731 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14732 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14734 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15460 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15461 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15462 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15755 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15756 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15757 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16424 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16425 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16425 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16539 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16539 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16539 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17563 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17563 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17563 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17622 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17622 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17622 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18001 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18001 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18152 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18152 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18153 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18605 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18605 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18605 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 18828 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 64512 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):name=cudaWords
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
half logThroughHelper(half value)
{
    return log(value);
}

[numthreads(128, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 91) return;
    uint errors = 0;
    half scalarResult = log(loadValue(lane));
    errors |= matchesExpected(scalarResult, lane) ? 0u : 1u;
    errors |= matchesExpected(logThroughHelper(loadValue(lane)), lane) ? 0u : 2u;
    half2 result2 = log(half2(loadValue((lane + 0) % 91), loadValue((lane + 1) % 91)));
    errors |= matchesExpected(result2.x, (lane + 0) % 91) ? 0u : 4u;
    errors |= matchesExpected(result2.y, (lane + 1) % 91) ? 0u : 4u;
    half3 result3 = log(half3(loadValue((lane + 0) % 91), loadValue((lane + 1) % 91), loadValue((lane + 2) % 91)));
    errors |= matchesExpected(result3.x, (lane + 0) % 91) ? 0u : 8u;
    errors |= matchesExpected(result3.y, (lane + 1) % 91) ? 0u : 8u;
    errors |= matchesExpected(result3.z, (lane + 2) % 91) ? 0u : 8u;
    half4 result4 = log(half4(loadValue((lane + 0) % 91), loadValue((lane + 1) % 91), loadValue((lane + 2) % 91), loadValue((lane + 3) % 91)));
    errors |= matchesExpected(result4.x, (lane + 0) % 91) ? 0u : 16u;
    errors |= matchesExpected(result4.y, (lane + 1) % 91) ? 0u : 16u;
    errors |= matchesExpected(result4.z, (lane + 2) % 91) ? 0u : 16u;
    errors |= matchesExpected(result4.w, (lane + 3) % 91) ? 0u : 16u;
    matrix<half, 2, 2> resultMatrix = log(matrix<half, 2, 2>(loadValue((lane + 0) % 91), loadValue((lane + 1) % 91), loadValue((lane + 2) % 91), loadValue((lane + 3) % 91)));
    errors |= matchesExpected(resultMatrix[0][0], (lane + 0) % 91) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[0][1], (lane + 1) % 91) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][0], (lane + 2) % 91) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][1], (lane + 3) % 91) ? 0u : 32u;
    uint low, high;
    valueBits(scalarResult, low, high);
    if (lane == 0) outputBuffer[1] = usesLibraryPolicy() ? 60043u : 1209u;
    outputBuffer[2 + 4 * lane] = errors;
    outputBuffer[3 + 4 * lane] = low;
    outputBuffer[4 + 4 * lane] = high;
    outputBuffer[5 + 4 * lane] = 60000 + lane;
}

// CUDA: 324508639{{$}}
// CUDA-NEXT: 1209{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60000{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60001{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60002{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60003{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60004{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60005{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60006{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60007{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60008{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60009{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60010{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60011{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60012{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60013{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60014{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60015{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60016{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60017{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60018{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60019{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60020{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60021{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60022{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60023{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60024{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60025{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60026{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60027{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60028{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60029{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60030{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60031{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60032{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60033{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60034{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60035{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60036{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60037{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60038{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60039{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60040{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60041{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60042{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60043{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60044{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60045{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60046{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60047{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60048{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60049{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60050{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60051{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60052{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60053{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60054{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60055{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60056{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60057{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60058{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60059{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60060{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60061{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60062{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60063{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60064{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60065{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60066{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60067{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60068{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60069{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60070{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60071{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60072{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60073{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60074{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60075{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60076{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60077{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60078{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60079{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60080{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60081{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60082{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60083{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60084{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60085{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60086{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60087{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60088{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60089{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 60090{{$}}
// CUDA-NEXT: 3735928559{{$}}
// NVVM: 324508639{{$}}
// NVVM-NEXT: 60043{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60000{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60001{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60002{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60003{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60004{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60005{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60006{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60007{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60008{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60009{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60010{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60011{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60012{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60013{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60014{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60015{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60016{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60017{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60018{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60019{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60020{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60021{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60022{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60023{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60024{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60025{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60026{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60027{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60028{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60029{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60030{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60031{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60032{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60033{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60034{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60035{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60036{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60037{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60038{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60039{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60040{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60041{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60042{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60043{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60044{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60045{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60046{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60047{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60048{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60049{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60050{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60051{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60052{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60053{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60054{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60055{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60056{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60057{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60058{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60059{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60060{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60061{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60062{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60063{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60064{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60065{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60066{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60067{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60068{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60069{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60070{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60071{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60072{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60073{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60074{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60075{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60076{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60077{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60078{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60079{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60080{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60081{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60082{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60083{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60084{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60085{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60086{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60087{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60088{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60089{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 60090{{$}}
// NVVM-NEXT: 3735928559{{$}}
