// Exact rational frac oracle: RN-even(x - floor(x)); finite bits include +0.
// NaNs and either infinity require NaN classification, not payload equality.
// Half keeps the whole Float32 expression; its finite result equals direct Half RN.
// 64 live lanes; 26 observations/lane across every public frac/fract shape.
// Eleven shape-error bits, two raw scalar results, completion, and endpoint guards.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT: ubuffer(data=[0 0 1 0 2 0 8388607 0 8388608 0 8388609 0 855638015 0 855638016 0 855638017 0 1056964607 0 1056964608 0 1056964609 0 1065353215 0 1065353216 0 1065353217 0 1073741823 0 1073741824 0 1073741825 0 1077936127 0 1077936128 0 1077936129 0 1258291199 0 1258291200 0 1258291201 0 1266679807 0 1266679808 0 1266679809 0 864026624 0 2139095039 0 2139095040 0 2143289379 0 2139095041 0 2147483648 0 2147483649 0 2147483650 0 2155872255 0 2155872256 0 2155872257 0 3003121663 0 3003121664 0 3003121665 0 3204448255 0 3204448256 0 3204448257 0 3212836863 0 3212836864 0 3212836865 0 3221225471 0 3221225472 0 3221225473 0 3225419775 0 3225419776 0 3225419777 0 3405774847 0 3405774848 0 3405774849 0 3414163455 0 3414163456 0 3414163457 0 3011510272 0 4286578687 0 4286578688 0 4290773027 0 4286578689 0], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 0 0 1 0 0 2 0 0 8388607 0 0 8388608 0 0 8388609 0 0 855638015 0 0 855638016 0 0 855638017 0 0 1056964607 0 0 1056964608 0 0 1056964609 0 0 1065353215 0 0 0 0 0 872415232 0 0 1065353214 0 0 0 0 0 880803840 0 0 1065353212 0 0 0 0 0 880803840 0 0 1056964608 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 864026624 0 0 0 0 0 0 0 1 0 0 1 0 0 1 0 0 0 1065353216 0 0 1065353216 0 0 1065353216 0 0 1065353216 0 0 1065353216 0 0 1065353216 0 0 1065353216 0 0 1065353215 0 0 1056964608 0 0 1056964608 0 0 1056964606 0 0 864026624 0 0 0 0 0 1065353214 0 0 872415232 0 0 0 0 0 1065353212 0 0 880803840 0 0 0 0 0 1065353212 0 0 1056964608 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1065353215 0 0 0 0 0 0 0 1 0 0 1 0 0 1], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

float loadValue(uint lane)
{
    return asfloat(inputWords[2 * lane]);
}

void valueBits(float value, out uint low, out uint high)
{
    low = asuint(value);
    high = 0;
}

bool matchesExpected(float value, uint lane)
{
    uint low, high;
    valueBits(value, low, high);
    bool nanBits = (low & 0x7f800000u) == 0x7f800000u && (low & 0x7fffffu) != 0;
    uint offset = 3 * lane;
    return expectedWords[offset + 2] != 0 ? nanBits :
        low == expectedWords[offset] && high == expectedWords[offset + 1];
}

[noinline]
float fracThroughHelper(float value)
{
    return frac(value);
}

[noinline]
float fractThroughHelper(float value)
{
    return fract(value);
}

[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 64) return;
    uint errors = 0;
    float fracScalar = frac(loadValue(lane));
    errors |= matchesExpected(fracScalar, lane) ? 0u : 1u;
    errors |= matchesExpected(fracThroughHelper(loadValue(lane)), lane) ? 0u : 2u;
    float2 fracResult2 = frac(float2(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64)));
    errors |= matchesExpected(fracResult2.x, (lane + 0) % 64) ? 0u : 4u;
    errors |= matchesExpected(fracResult2.y, (lane + 1) % 64) ? 0u : 4u;
    float3 fracResult3 = frac(float3(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64)));
    errors |= matchesExpected(fracResult3.x, (lane + 0) % 64) ? 0u : 8u;
    errors |= matchesExpected(fracResult3.y, (lane + 1) % 64) ? 0u : 8u;
    errors |= matchesExpected(fracResult3.z, (lane + 2) % 64) ? 0u : 8u;
    float4 fracResult4 = frac(float4(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(fracResult4.x, (lane + 0) % 64) ? 0u : 16u;
    errors |= matchesExpected(fracResult4.y, (lane + 1) % 64) ? 0u : 16u;
    errors |= matchesExpected(fracResult4.z, (lane + 2) % 64) ? 0u : 16u;
    errors |= matchesExpected(fracResult4.w, (lane + 3) % 64) ? 0u : 16u;
    matrix<float, 2, 2> resultMatrix = frac(matrix<float, 2, 2>(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(resultMatrix[0][0], (lane + 0) % 64) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[0][1], (lane + 1) % 64) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][0], (lane + 2) % 64) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][1], (lane + 3) % 64) ? 0u : 32u;
    float fractScalar = fract(loadValue(lane));
    errors |= matchesExpected(fractScalar, lane) ? 0u : 64u;
    errors |= matchesExpected(fractThroughHelper(loadValue(lane)), lane) ? 0u : 128u;
    float2 fractResult2 = fract(float2(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64)));
    errors |= matchesExpected(fractResult2.x, (lane + 0) % 64) ? 0u : 256u;
    errors |= matchesExpected(fractResult2.y, (lane + 1) % 64) ? 0u : 256u;
    float3 fractResult3 = fract(float3(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64)));
    errors |= matchesExpected(fractResult3.x, (lane + 0) % 64) ? 0u : 512u;
    errors |= matchesExpected(fractResult3.y, (lane + 1) % 64) ? 0u : 512u;
    errors |= matchesExpected(fractResult3.z, (lane + 2) % 64) ? 0u : 512u;
    float4 fractResult4 = fract(float4(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
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
// CHECK-NEXT: 8388607{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 8388607{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59003{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 8388608{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 8388608{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59004{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 8388609{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 8388609{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59005{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 855638015{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 855638015{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59006{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 855638016{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 855638016{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59007{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 855638017{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 855638017{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59008{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1056964607{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1056964607{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59009{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1056964608{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1056964608{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59010{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1056964609{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1056964609{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59011{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353215{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353215{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59012{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59013{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 872415232{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 872415232{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59014{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353214{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353214{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59015{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59016{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 880803840{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 880803840{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59017{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353212{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353212{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59018{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59019{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 880803840{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 880803840{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59020{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1056964608{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1056964608{{$}}
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
// CHECK-NEXT: 864026624{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 864026624{{$}}
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
// CHECK-NEXT: 1065353216{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353216{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59033{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353216{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353216{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59034{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353216{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353216{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59035{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353216{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353216{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59036{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353216{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353216{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59037{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353216{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353216{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59038{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353216{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353216{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59039{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353215{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353215{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59040{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1056964608{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1056964608{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59041{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1056964608{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1056964608{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59042{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1056964606{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1056964606{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59043{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 864026624{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 864026624{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59044{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59045{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353214{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353214{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59046{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 872415232{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 872415232{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59047{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59048{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353212{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353212{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59049{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 880803840{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 880803840{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59050{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59051{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353212{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353212{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59052{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1056964608{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1056964608{{$}}
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
// CHECK-NEXT: 1065353215{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1065353215{{$}}
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
