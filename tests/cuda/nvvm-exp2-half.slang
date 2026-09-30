// Exp2 admission uses an explicitly test-defined spacing/encoding union, not a vendor ULP metric.
// Library N=2 for Float32 and N=1 for Float64 is an empirical, non-guaranteed qualification.
// Zero uses minsubnormal spacing; maxfinite uses the conceptual next binade; infinity uses
// encoding steps only. Special inputs have exact classification rules overriding that union.
// NVVM Half narrows the selected Float32 library set once. CUDA Half follows the installed
// ex2.approx.ftz/FMA.rn32(q,2^-24,q)/narrow sequence and has its own discrete candidate set.
// References are certified from exact input bits before observation; NaN payloads remain open.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CUDA): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=NVVM): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=NVVM): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT: ubuffer(data=[0 0 1 0 2 0 1023 0 1024 0 1025 0 4547 0 4548 0 4549 0 4550 0 6226 0 6227 0 6228 0 6229 0 13312 0 14336 0 15358 0 15359 0 15360 0 15361 0 16382 0 16383 0 16384 0 16385 0 17408 0 18432 0 19328 0 19454 0 19455 0 19456 0 19457 0 22527 0 22528 0 31743 0 31744 0 31745 0 32256 0 32768 0 32769 0 32770 0 33791 0 33792 0 33793 0 46080 0 47104 0 48127 0 48128 0 48129 0 48130 0 49152 0 50176 0 51200 0 51967 0 51968 0 51969 0 51970 0 52224 0 52736 0 52799 0 52800 0 52801 0 52802 0 53248 0 54272 0 54848 0 55264 0 55265 0 55472 0 55473 0 57720 0 64511 0 64512 0 64513 0 65024 0], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 15360 0 15361 0 0 0 0 0 0 0 0 0 0 0 0 1 15361 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15361 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 15361 0 15362 0 0 0 0 0 0 0 0 0 0 0 0 1 15362 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15362 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15554 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15784 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16383 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16383 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16384 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16385 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17405 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17407 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17408 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17409 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 19456 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 23552 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 30720 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31722 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31733 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15034 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14760 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14336 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14336 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14335 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14333 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13312 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 11264 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 7168 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1030 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1024 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1018 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1013 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 256 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 15360 0 15361 0 0 0 0 0 0 0 0 0 0 0 0 1 15361 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15361 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 15361 0 15362 0 0 0 0 0 0 0 0 0 0 0 0 1 15362 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15362 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15554 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15784 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16383 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16383 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16384 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 16385 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17405 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17407 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17408 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 17409 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 19456 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 23552 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 30720 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31722 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31733 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 31744 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15360 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 15034 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14760 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14336 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14336 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14335 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 14333 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 13312 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 11264 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 7168 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1030 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1024 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1018 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1013 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 256 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 2 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):name=cudaWords
StructuredBuffer<uint> cudaWords;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
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
half exp2ThroughHelper(half value)
{
    return exp2(value);
}

[numthreads(128, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 74) return;
    uint errors = 0;
    half scalarResult = exp2(loadValue(lane));
    errors |= matchesExpected(scalarResult, lane) ? 0u : 1u;
    errors |= matchesExpected(exp2ThroughHelper(loadValue(lane)), lane) ? 0u : 2u;
    half2 result2 = exp2(half2(loadValue((lane + 0) % 74), loadValue((lane + 1) % 74)));
    errors |= matchesExpected(result2.x, (lane + 0) % 74) ? 0u : 4u;
    errors |= matchesExpected(result2.y, (lane + 1) % 74) ? 0u : 4u;
    half3 result3 = exp2(half3(loadValue((lane + 0) % 74), loadValue((lane + 1) % 74), loadValue((lane + 2) % 74)));
    errors |= matchesExpected(result3.x, (lane + 0) % 74) ? 0u : 8u;
    errors |= matchesExpected(result3.y, (lane + 1) % 74) ? 0u : 8u;
    errors |= matchesExpected(result3.z, (lane + 2) % 74) ? 0u : 8u;
    half4 result4 = exp2(half4(loadValue((lane + 0) % 74), loadValue((lane + 1) % 74), loadValue((lane + 2) % 74), loadValue((lane + 3) % 74)));
    errors |= matchesExpected(result4.x, (lane + 0) % 74) ? 0u : 16u;
    errors |= matchesExpected(result4.y, (lane + 1) % 74) ? 0u : 16u;
    errors |= matchesExpected(result4.z, (lane + 2) % 74) ? 0u : 16u;
    errors |= matchesExpected(result4.w, (lane + 3) % 74) ? 0u : 16u;
    matrix<half, 2, 2> resultMatrix = exp2(matrix<half, 2, 2>(loadValue((lane + 0) % 74), loadValue((lane + 1) % 74), loadValue((lane + 2) % 74), loadValue((lane + 3) % 74)));
    errors |= matchesExpected(resultMatrix[0][0], (lane + 0) % 74) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[0][1], (lane + 1) % 74) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][0], (lane + 2) % 74) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][1], (lane + 3) % 74) ? 0u : 32u;
    uint low, high;
    valueBits(scalarResult, low, high);
    if (lane == 0) outputBuffer[1] = usesLibraryPolicy() ? 56042u : 1209u;
    outputBuffer[2 + 4 * lane] = errors;
    outputBuffer[3 + 4 * lane] = low;
    outputBuffer[4 + 4 * lane] = high;
    outputBuffer[5 + 4 * lane] = 56000 + lane;
}

// CUDA: 324508639{{$}}
// CUDA-NEXT: 1209{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56000{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56001{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56002{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56003{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56004{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56005{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56006{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56007{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56008{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56009{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56010{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56011{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56012{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56013{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56014{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56015{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56016{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56017{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56018{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56019{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56020{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56021{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56022{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56023{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56024{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56025{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56026{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56027{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56028{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56029{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56030{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56031{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56032{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56033{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56034{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56035{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56036{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56037{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56038{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56039{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56040{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56041{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56042{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56043{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56044{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56045{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56046{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56047{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56048{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56049{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56050{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56051{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56052{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56053{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56054{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56055{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56056{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56057{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56058{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56059{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56060{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56061{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56062{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56063{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56064{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56065{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56066{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56067{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56068{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56069{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56070{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56071{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56072{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 56073{{$}}
// CUDA-NEXT: 3735928559{{$}}
// NVVM: 324508639{{$}}
// NVVM-NEXT: 56042{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56000{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56001{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56002{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56003{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56004{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56005{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56006{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56007{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56008{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56009{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56010{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56011{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56012{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56013{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56014{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56015{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56016{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56017{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56018{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56019{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56020{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56021{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56022{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56023{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56024{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56025{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56026{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56027{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56028{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56029{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56030{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56031{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56032{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56033{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56034{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56035{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56036{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56037{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56038{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56039{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56040{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56041{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56042{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56043{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56044{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56045{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56046{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56047{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56048{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56049{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56050{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56051{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56052{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56053{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56054{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56055{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56056{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56057{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56058{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56059{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56060{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56061{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56062{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56063{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56064{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56065{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56066{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56067{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56068{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56069{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56070{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56071{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56072{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 56073{{$}}
// NVVM-NEXT: 3735928559{{$}}
