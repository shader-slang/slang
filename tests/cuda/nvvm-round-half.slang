// Both oracle comparisons execute unconditionally on every backend.
// NVVM preserves the existing Half -> roundf -> Half ties-away recipe.
// CUDA Half keeps hrint, which rounds ties to even. No shader target selects the oracle.
// NaNs compare IEEE class only; all other results compare exact Half bits.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CUDA): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=NVVM): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=NVVM): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT: ubuffer(data=[0 13312 14335 14336 14337 15871 15872 15873 16639 16640 16641 15360 16384 1 1023 1024 25599 25600 25601 26623 26624 26625 31743 31744 32547 31745 32768 46080 47103 47104 47105 48639 48640 48641 49407 49408 49409 48128 49152 32769 33791 33792 58367 58368 58369 59391 59392 59393 64511 64512 65315 64513], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 15360 0 0 15360 15360 0 15360 15360 0 16384 16384 0 16384 16384 0 16384 16384 0 16896 16384 0 16896 16896 0 15360 15360 0 16384 16384 0 0 0 0 0 0 0 0 0 0 25600 25600 0 25600 25600 0 25601 25601 0 26623 26623 0 26624 26624 0 26625 26625 0 31743 31743 0 31744 31744 0 0 0 1 0 0 1 32768 32768 0 32768 32768 0 32768 32768 0 48128 32768 0 48128 48128 0 48128 48128 0 49152 49152 0 49152 49152 0 49152 49152 0 49664 49152 0 49664 49664 0 48128 48128 0 49152 49152 0 32768 32768 0 32768 32768 0 32768 32768 0 58368 58368 0 58368 58368 0 58369 58369 0 59391 59391 0 59392 59392 0 59393 59393 0 64511 64511 0 64512 64512 0 0 0 1 0 0 1], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

half loadValue(uint lane)
{
    return bit_cast<half>(uint16_t(inputWords[lane]));
}

uint2 mismatchPair(half actual, uint lane)
{
    uint bits = uint(bit_cast<uint16_t>(actual));
    bool nanBits = (bits & 0x7c00u) == 0x7c00u && (bits & 0x3ffu) != 0;
    bool awayOK = expectedWords[3 * lane + 2] != 0 ? nanBits :
        bits == expectedWords[3 * lane];
    bool evenOK = expectedWords[3 * lane + 2] != 0 ? nanBits :
        bits == expectedWords[3 * lane + 1];
    return uint2(awayOK ? 0u : 1u, evenOK ? 0u : 1u);
}

[noinline]
half roundThroughHelper(half value)
{
    return round(value);
}

[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 52) return;
    uint2 errors = mismatchPair(round(loadValue(lane)), lane);
    errors |= mismatchPair(roundThroughHelper(loadValue(lane)), lane) * 2u;
    half2 result2 = round(half2(loadValue((lane + 0) % 52), loadValue((lane + 1) % 52)));
    errors |= mismatchPair(result2.x, (lane + 0) % 52) * 4u;
    errors |= mismatchPair(result2.y, (lane + 1) % 52) * 4u;
    half3 result3 = round(half3(loadValue((lane + 0) % 52), loadValue((lane + 1) % 52), loadValue((lane + 2) % 52)));
    errors |= mismatchPair(result3.x, (lane + 0) % 52) * 8u;
    errors |= mismatchPair(result3.y, (lane + 1) % 52) * 8u;
    errors |= mismatchPair(result3.z, (lane + 2) % 52) * 8u;
    half4 result4 = round(half4(loadValue((lane + 0) % 52), loadValue((lane + 1) % 52), loadValue((lane + 2) % 52), loadValue((lane + 3) % 52)));
    errors |= mismatchPair(result4.x, (lane + 0) % 52) * 16u;
    errors |= mismatchPair(result4.y, (lane + 1) % 52) * 16u;
    errors |= mismatchPair(result4.z, (lane + 2) % 52) * 16u;
    errors |= mismatchPair(result4.w, (lane + 3) % 52) * 16u;
    matrix<half, 2, 2> matrixResult = round(matrix<half, 2, 2>(loadValue((lane + 0) % 52), loadValue((lane + 1) % 52), loadValue((lane + 2) % 52), loadValue((lane + 3) % 52)));
    errors |= mismatchPair(matrixResult[0][0], (lane + 0) % 52) * 32u;
    errors |= mismatchPair(matrixResult[0][1], (lane + 1) % 52) * 32u;
    errors |= mismatchPair(matrixResult[1][0], (lane + 2) % 52) * 32u;
    errors |= mismatchPair(matrixResult[1][1], (lane + 3) % 52) * 32u;
    outputBuffer[1 + 3 * lane] = errors.x;
    outputBuffer[2 + 3 * lane] = errors.y;
    outputBuffer[3 + 3 * lane] = 12000 + lane;
}

// CUDA: 324508639{{$}}
// CUDA-NEXT: 48{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12000{{$}}
// CUDA-NEXT: 56{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12001{{$}}
// CUDA-NEXT: 60{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12002{{$}}
// CUDA-NEXT: 63{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12003{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12004{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12005{{$}}
// CUDA-NEXT: 48{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12006{{$}}
// CUDA-NEXT: 56{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12007{{$}}
// CUDA-NEXT: 60{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12008{{$}}
// CUDA-NEXT: 63{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12009{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12010{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12011{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12012{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12013{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12014{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12015{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12016{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12017{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12018{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12019{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12020{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12021{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12022{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12023{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12024{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12025{{$}}
// CUDA-NEXT: 48{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12026{{$}}
// CUDA-NEXT: 56{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12027{{$}}
// CUDA-NEXT: 60{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12028{{$}}
// CUDA-NEXT: 63{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12029{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12030{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12031{{$}}
// CUDA-NEXT: 48{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12032{{$}}
// CUDA-NEXT: 56{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12033{{$}}
// CUDA-NEXT: 60{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12034{{$}}
// CUDA-NEXT: 63{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12035{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12036{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12037{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12038{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12039{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12040{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12041{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12042{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12043{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12044{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12045{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12046{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12047{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12048{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12049{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12050{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 12051{{$}}
// CUDA-NEXT: 3735928559{{$}}

// NVVM: 324508639{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 48{{$}}
// NVVM-NEXT: 12000{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 56{{$}}
// NVVM-NEXT: 12001{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 60{{$}}
// NVVM-NEXT: 12002{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 63{{$}}
// NVVM-NEXT: 12003{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12004{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12005{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 48{{$}}
// NVVM-NEXT: 12006{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 56{{$}}
// NVVM-NEXT: 12007{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 60{{$}}
// NVVM-NEXT: 12008{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 63{{$}}
// NVVM-NEXT: 12009{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12010{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12011{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12012{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12013{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12014{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12015{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12016{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12017{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12018{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12019{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12020{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12021{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12022{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12023{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12024{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12025{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 48{{$}}
// NVVM-NEXT: 12026{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 56{{$}}
// NVVM-NEXT: 12027{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 60{{$}}
// NVVM-NEXT: 12028{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 63{{$}}
// NVVM-NEXT: 12029{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12030{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12031{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 48{{$}}
// NVVM-NEXT: 12032{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 56{{$}}
// NVVM-NEXT: 12033{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 60{{$}}
// NVVM-NEXT: 12034{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 63{{$}}
// NVVM-NEXT: 12035{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12036{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12037{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12038{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12039{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12040{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12041{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12042{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12043{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12044{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12045{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12046{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12047{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12048{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12049{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12050{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 12051{{$}}
// NVVM-NEXT: 3735928559{{$}}
