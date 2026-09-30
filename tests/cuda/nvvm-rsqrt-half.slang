// Rsqrt library admission is an empirical, non-guaranteed scoped qualification policy.
// It is the union of N times the reference's maximum adjacent spacing and N encoding steps,
// centered on the exact same-width RN-even reference (Float32 N=2, Float64 N=1).
// This explicit union is not a vendor-defined ULP metric or a correct-rounding claim.
// NVVM Half is the exact RN16 image of that Float32 set; CUDA Half is the exact RN16 image
// of Float32 candidates satisfying PTX epsilon=2^-22.9, certified by rational tenth powers.
// 64 live inputs, 15 observations per lane; six shape-error bits, raw scalar bits and completion.
// NaN classification leaves payloads open; every raw before/after buffer remains a separate gate.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CUDA): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=NVVM): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=NVVM): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT: ubuffer(data=[0 0 1 0 2 0 3 0 1022 0 1023 0 1024 0 1025 0 31742 0 31743 0 31744 0 32547 0 31745 0 13311 0 13312 0 13313 0 15359 0 15360 0 15361 0 16383 0 16384 0 16385 0 17407 0 17408 0 17409 0 18559 0 18560 0 18561 0 15872 0 12409 0 14188 0 14457 0 32768 0 32769 0 32770 0 32771 0 33790 0 33791 0 33792 0 33793 0 64510 0 64511 0 64512 0 65315 0 64513 0 46079 0 46080 0 46081 0 48127 0 48128 0 48129 0 49151 0 49152 0 49153 0 50175 0 50176 0 50177 0 51327 0 51328 0 51329 0 48640 0 45177 0 46956 0 47225 0], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 27648 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 27048 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 26782 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 22529 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 22529 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 22528 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 22527 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 7169 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 7168 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16384 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16384 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16383 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15359 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14761 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14760 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14759 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14336 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14336 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14335 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13654 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13653 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13653 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14984 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16730 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15839 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15706 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 64512 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 27648 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 27048 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 26782 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 22529 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 22529 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 22528 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 22527 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 7169 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 7168 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16384 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16384 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16383 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15359 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14761 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14760 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14759 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14336 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14336 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14335 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13654 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13653 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13653 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14984 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16730 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15839 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15706 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 64512 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):name=cudaWords
StructuredBuffer<uint> cudaWords;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
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
half rsqrtThroughHelper(half value)
{
    return rsqrt(value);
}

[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 64) return;
    uint errors = 0;
    half scalarResult = rsqrt(loadValue(lane));
    errors |= matchesExpected(scalarResult, lane) ? 0u : 1u;
    errors |= matchesExpected(rsqrtThroughHelper(loadValue(lane)), lane) ? 0u : 2u;
    half2 result2 = rsqrt(half2(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64)));
    errors |= matchesExpected(result2.x, (lane + 0) % 64) ? 0u : 4u;
    errors |= matchesExpected(result2.y, (lane + 1) % 64) ? 0u : 4u;
    half3 result3 = rsqrt(half3(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64)));
    errors |= matchesExpected(result3.x, (lane + 0) % 64) ? 0u : 8u;
    errors |= matchesExpected(result3.y, (lane + 1) % 64) ? 0u : 8u;
    errors |= matchesExpected(result3.z, (lane + 2) % 64) ? 0u : 8u;
    half4 result4 = rsqrt(half4(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(result4.x, (lane + 0) % 64) ? 0u : 16u;
    errors |= matchesExpected(result4.y, (lane + 1) % 64) ? 0u : 16u;
    errors |= matchesExpected(result4.z, (lane + 2) % 64) ? 0u : 16u;
    errors |= matchesExpected(result4.w, (lane + 3) % 64) ? 0u : 16u;
    matrix<half, 2, 2> resultMatrix = rsqrt(matrix<half, 2, 2>(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(resultMatrix[0][0], (lane + 0) % 64) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[0][1], (lane + 1) % 64) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][0], (lane + 2) % 64) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][1], (lane + 3) % 64) ? 0u : 32u;
    uint low, high;
    valueBits(scalarResult, low, high);
    if (lane == 0) outputBuffer[1] = usesLibraryPolicy() ? 65039u : 1209u;
    outputBuffer[2 + 4 * lane] = errors;
    outputBuffer[3 + 4 * lane] = low;
    outputBuffer[4 + 4 * lane] = high;
    outputBuffer[5 + 4 * lane] = 65000 + lane;
}

// CUDA: 324508639{{$}}
// CUDA-NEXT: 1209{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65000{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65001{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65002{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65003{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65004{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65005{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65006{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65007{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65008{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65009{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65010{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65011{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65012{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65013{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65014{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65015{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65016{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65017{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65018{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65019{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65020{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65021{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65022{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65023{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65024{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65025{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65026{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65027{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65028{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65029{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65030{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65031{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65032{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65033{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65034{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65035{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65036{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65037{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65038{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65039{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65040{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65041{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65042{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65043{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65044{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65045{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65046{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65047{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65048{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65049{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65050{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65051{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65052{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65053{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65054{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65055{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65056{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65057{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65058{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65059{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65060{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65061{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65062{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 65063{{$}}
// CUDA-NEXT: 3735928559{{$}}
// NVVM: 324508639{{$}}
// NVVM-NEXT: 65039{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65000{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65001{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65002{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65003{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65004{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65005{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65006{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65007{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65008{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65009{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65010{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65011{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65012{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65013{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65014{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65015{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65016{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65017{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65018{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65019{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65020{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65021{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65022{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65023{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65024{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65025{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65026{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65027{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65028{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65029{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65030{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65031{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65032{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65033{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65034{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65035{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65036{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65037{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65038{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65039{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65040{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65041{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65042{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65043{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65044{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65045{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65046{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65047{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65048{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65049{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65050{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65051{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65052{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65053{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65054{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65055{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65056{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65057{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65058{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65059{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65060{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65061{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65062{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 65063{{$}}
// NVVM-NEXT: 3735928559{{$}}
