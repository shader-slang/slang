// Exact rational frac oracle: RN-even(x - floor(x)); finite bits include +0.
// NaNs and either infinity require NaN classification, not payload equality.
// Half keeps the whole Float32 expression; its finite result equals direct Half RN.
// 64 live lanes; 26 observations/lane across every public frac/fract shape.
// Eleven shape-error bits, two raw scalar results, completion, and endpoint guards.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT: ubuffer(data=[0 0 1 0 2 0 1023 0 1024 0 1025 0 3071 0 3072 0 3073 0 14335 0 14336 0 14337 0 15359 0 15360 0 15361 0 16383 0 16384 0 16385 0 16895 0 16896 0 16897 0 25599 0 25600 0 25601 0 26623 0 26624 0 26625 0 4096 0 31743 0 31744 0 32291 0 31745 0 32768 0 32769 0 32770 0 33791 0 33792 0 33793 0 35839 0 35840 0 35841 0 47103 0 47104 0 47105 0 48127 0 48128 0 48129 0 49151 0 49152 0 49153 0 49663 0 49664 0 49665 0 58367 0 58368 0 58369 0 59391 0 59392 0 59393 0 36864 0 64511 0 64512 0 65059 0 64513 0], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 0 0 1 0 0 2 0 0 1023 0 0 1024 0 0 1025 0 0 3071 0 0 3072 0 0 3073 0 0 14335 0 0 14336 0 0 14337 0 0 15359 0 0 0 0 0 5120 0 0 15358 0 0 0 0 0 6144 0 0 15356 0 0 0 0 0 6144 0 0 14336 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 4096 0 0 0 0 0 0 0 1 0 0 1 0 0 1 0 0 0 15360 0 0 15360 0 0 15360 0 0 15360 0 0 15360 0 0 15360 0 0 15360 0 0 15359 0 0 14336 0 0 14336 0 0 14334 0 0 4096 0 0 0 0 0 15358 0 0 5120 0 0 0 0 0 15356 0 0 6144 0 0 0 0 0 15356 0 0 14336 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 15359 0 0 0 0 0 0 0 1 0 0 1 0 0 1], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
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

bool matchesExpected(half value, uint lane)
{
    uint low, high;
    valueBits(value, low, high);
    bool nanBits = (low & 0x7c00u) == 0x7c00u && (low & 0x3ffu) != 0;
    uint offset = 3 * lane;
    return expectedWords[offset + 2] != 0 ? nanBits :
        low == expectedWords[offset] && high == expectedWords[offset + 1];
}

[noinline]
half fracThroughHelper(half value)
{
    return frac(value);
}

[noinline]
half fractThroughHelper(half value)
{
    return fract(value);
}

[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 64) return;
    uint errors = 0;
    half fracScalar = frac(loadValue(lane));
    errors |= matchesExpected(fracScalar, lane) ? 0u : 1u;
    errors |= matchesExpected(fracThroughHelper(loadValue(lane)), lane) ? 0u : 2u;
    half2 fracResult2 = frac(half2(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64)));
    errors |= matchesExpected(fracResult2.x, (lane + 0) % 64) ? 0u : 4u;
    errors |= matchesExpected(fracResult2.y, (lane + 1) % 64) ? 0u : 4u;
    half3 fracResult3 = frac(half3(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64)));
    errors |= matchesExpected(fracResult3.x, (lane + 0) % 64) ? 0u : 8u;
    errors |= matchesExpected(fracResult3.y, (lane + 1) % 64) ? 0u : 8u;
    errors |= matchesExpected(fracResult3.z, (lane + 2) % 64) ? 0u : 8u;
    half4 fracResult4 = frac(half4(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(fracResult4.x, (lane + 0) % 64) ? 0u : 16u;
    errors |= matchesExpected(fracResult4.y, (lane + 1) % 64) ? 0u : 16u;
    errors |= matchesExpected(fracResult4.z, (lane + 2) % 64) ? 0u : 16u;
    errors |= matchesExpected(fracResult4.w, (lane + 3) % 64) ? 0u : 16u;
    matrix<half, 2, 2> resultMatrix = frac(matrix<half, 2, 2>(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(resultMatrix[0][0], (lane + 0) % 64) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[0][1], (lane + 1) % 64) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][0], (lane + 2) % 64) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][1], (lane + 3) % 64) ? 0u : 32u;
    half fractScalar = fract(loadValue(lane));
    errors |= matchesExpected(fractScalar, lane) ? 0u : 64u;
    errors |= matchesExpected(fractThroughHelper(loadValue(lane)), lane) ? 0u : 128u;
    half2 fractResult2 = fract(half2(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64)));
    errors |= matchesExpected(fractResult2.x, (lane + 0) % 64) ? 0u : 256u;
    errors |= matchesExpected(fractResult2.y, (lane + 1) % 64) ? 0u : 256u;
    half3 fractResult3 = fract(half3(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64)));
    errors |= matchesExpected(fractResult3.x, (lane + 0) % 64) ? 0u : 512u;
    errors |= matchesExpected(fractResult3.y, (lane + 1) % 64) ? 0u : 512u;
    errors |= matchesExpected(fractResult3.z, (lane + 2) % 64) ? 0u : 512u;
    half4 fractResult4 = fract(half4(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(fractResult4.x, (lane + 0) % 64) ? 0u : 1024u;
    errors |= matchesExpected(fractResult4.y, (lane + 1) % 64) ? 0u : 1024u;
    errors |= matchesExpected(fractResult4.z, (lane + 2) % 64) ? 0u : 1024u;
    errors |= matchesExpected(fractResult4.w, (lane + 3) % 64) ? 0u : 1024u;
    uint fracLow, fracHigh, fractLow, fractHigh;
    valueBits(fracScalar, fracLow, fracHigh);
    valueBits(fractScalar, fractLow, fractHigh);
    outputBuffer[1 + 6 * lane] = errors;
    outputBuffer[2 + 6 * lane] = fracLow;
    outputBuffer[3 + 6 * lane] = fracHigh;
    outputBuffer[4 + 6 * lane] = fractLow;
    outputBuffer[5 + 6 * lane] = fractHigh;
    outputBuffer[6 + 6 * lane] = 59000 + lane;
}

// CHECK: 324508639{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59000{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59001{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 2{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 2{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59002{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1023{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1023{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59003{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1024{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1024{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59004{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1025{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1025{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59005{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 3071{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 3071{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59006{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 3072{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 3072{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59007{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 3073{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 3073{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59008{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 14335{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 14335{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59009{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 14336{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 14336{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59010{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 14337{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 14337{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59011{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15359{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15359{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59012{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59013{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 5120{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 5120{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59014{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15358{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15358{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59015{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59016{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 6144{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 6144{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59017{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15356{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15356{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59018{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59019{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 6144{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 6144{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59020{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 14336{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 14336{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59021{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59022{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59023{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59024{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59025{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59026{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 4096{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 4096{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59027{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59028{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 59029{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 59030{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 59031{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59032{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59033{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59034{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59035{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59036{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59037{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59038{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15360{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59039{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15359{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15359{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59040{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 14336{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 14336{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59041{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 14336{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 14336{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59042{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 14334{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 14334{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59043{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 4096{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 4096{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59044{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59045{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15358{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15358{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59046{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 5120{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 5120{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59047{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59048{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15356{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15356{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59049{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 6144{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 6144{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59050{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59051{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15356{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15356{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59052{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 14336{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 14336{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59053{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59054{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59055{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59056{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59057{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59058{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15359{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 15359{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59059{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59060{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 59061{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 59062{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 59063{{$}}
// CHECK-NEXT: 3735928559{{$}}
