// Frozen input/oracle constants generated independently of compiler execution.
// Rounding: nearest integer, ties away from zero; preserve signed zero and infinities.
// NaNs require only an IEEE NaN encoding, with no sign/payload promise.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT: ubuffer(data=[0 0 1048576000 0 1056964607 0 1056964608 0 1056964609 0 1069547519 0 1069547520 0 1069547521 0 1075838975 0 1075838976 0 1075838977 0 1065353216 0 1073741824 0 1 0 8388607 0 8388608 0 1258291199 0 1258291200 0 1258291201 0 1266679807 0 1266679808 0 1266679809 0 2139095039 0 2139095040 0 2143289635 0 2139095041 0 2147483648 0 3196059648 0 3204448255 0 3204448256 0 3204448257 0 3217031167 0 3217031168 0 3217031169 0 3223322623 0 3223322624 0 3223322625 0 3212836864 0 3221225472 0 2147483649 0 2155872255 0 2155872256 0 3405774847 0 3405774848 0 3405774849 0 3414163455 0 3414163456 0 3414163457 0 4286578687 0 4286578688 0 4290773283 0 4286578689 0], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 1065353216 0 0 1065353216 0 0 1065353216 0 0 1073741824 0 0 1073741824 0 0 1073741824 0 0 1077936128 0 0 1077936128 0 0 1065353216 0 0 1073741824 0 0 0 0 0 0 0 0 0 0 0 1258291200 0 0 1258291200 0 0 1258291201 0 0 1266679807 0 0 1266679808 0 0 1266679809 0 0 2139095039 0 0 2139095040 0 0 0 0 1 0 0 1 2147483648 0 0 2147483648 0 0 2147483648 0 0 3212836864 0 0 3212836864 0 0 3212836864 0 0 3221225472 0 0 3221225472 0 0 3221225472 0 0 3225419776 0 0 3225419776 0 0 3212836864 0 0 3221225472 0 0 2147483648 0 0 2147483648 0 0 2147483648 0 0 3405774848 0 0 3405774848 0 0 3405774849 0 0 3414163455 0 0 3414163456 0 0 3414163457 0 0 4286578687 0 0 4286578688 0 0 0 0 1 0 0 1], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

float loadValue(uint lane)
{
    return asfloat(inputWords[2 * lane]);
}

bool matchesExpected(float value, uint lane)
{
    uint low = asuint(value);
    bool nanBits = (low & 0x7f800000u) == 0x7f800000u && (low & 0x007fffffu) != 0;
    return expectedWords[3 * lane + 2] != 0 ? nanBits : low == expectedWords[3 * lane];
}

[noinline]
float roundThroughHelper(float value)
{
    return round(value);
}

[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 52) return;
    uint errors = 0;
    errors |= matchesExpected(round(loadValue(lane)), lane) ? 0u : 1u;
    errors |= matchesExpected(roundThroughHelper(loadValue(lane)), lane) ? 0u : 2u;
    float2 result2 = round(float2(loadValue((lane + 0) % 52), loadValue((lane + 1) % 52)));
    errors |= matchesExpected(result2.x, (lane + 0) % 52) ? 0u : 4u;
    errors |= matchesExpected(result2.y, (lane + 1) % 52) ? 0u : 4u;
    float3 result3 = round(float3(loadValue((lane + 0) % 52), loadValue((lane + 1) % 52), loadValue((lane + 2) % 52)));
    errors |= matchesExpected(result3.x, (lane + 0) % 52) ? 0u : 8u;
    errors |= matchesExpected(result3.y, (lane + 1) % 52) ? 0u : 8u;
    errors |= matchesExpected(result3.z, (lane + 2) % 52) ? 0u : 8u;
    float4 result4 = round(float4(loadValue((lane + 0) % 52), loadValue((lane + 1) % 52), loadValue((lane + 2) % 52), loadValue((lane + 3) % 52)));
    errors |= matchesExpected(result4.x, (lane + 0) % 52) ? 0u : 16u;
    errors |= matchesExpected(result4.y, (lane + 1) % 52) ? 0u : 16u;
    errors |= matchesExpected(result4.z, (lane + 2) % 52) ? 0u : 16u;
    errors |= matchesExpected(result4.w, (lane + 3) % 52) ? 0u : 16u;
    matrix<float, 2, 2> matrixResult = round(matrix<float, 2, 2>(loadValue((lane + 0) % 52), loadValue((lane + 1) % 52), loadValue((lane + 2) % 52), loadValue((lane + 3) % 52)));
    errors |= matchesExpected(matrixResult[0][0], (lane + 0) % 52) ? 0u : 32u;
    errors |= matchesExpected(matrixResult[0][1], (lane + 1) % 52) ? 0u : 32u;
    errors |= matchesExpected(matrixResult[1][0], (lane + 2) % 52) ? 0u : 32u;
    errors |= matchesExpected(matrixResult[1][1], (lane + 3) % 52) ? 0u : 32u;
    outputBuffer[1 + 2 * lane] = errors;
    outputBuffer[2 + 2 * lane] = 10000 + lane;
}

// CHECK: 324508639{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10000{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10001{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10002{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10003{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10004{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10005{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10006{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10007{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10008{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10009{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10010{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10011{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10012{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10013{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10014{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10015{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10016{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10017{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10018{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10019{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10020{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10021{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10022{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10023{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10024{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10025{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10026{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10027{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10028{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10029{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10030{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10031{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10032{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10033{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10034{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10035{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10036{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10037{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10038{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10039{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10040{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10041{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10042{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10043{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10044{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10045{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10046{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10047{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10048{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10049{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10050{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 10051{{$}}
// CHECK-NEXT: 3735928559{{$}}
