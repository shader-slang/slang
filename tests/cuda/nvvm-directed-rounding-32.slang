// Exact independent IEEE input/oracle constants; no target-dependent oracle selection.
// Ceil rounds toward +infinity, floor toward -infinity, trunc toward zero.
// Preserve signed zeros and infinities; compare NaNs by class only.
// Each output mask reserves six bits per operation: scalar, helper, vector2/3/4, matrix2x2.
// 64 live lanes exercise 45 results each; completion words and end guards are checked.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT: ubuffer(data=[0 0 1 0 8388606 0 8388607 0 8388608 0 8388609 0 1048576000 0 1056964607 0 1056964608 0 1056964609 0 1065353215 0 1065353216 0 1065353217 0 1069547519 0 1069547520 0 1069547521 0 1073741823 0 1073741824 0 1073741825 0 1075838975 0 1075838976 0 1075838977 0 1249902591 0 1249902592 0 1249902593 0 1258291199 0 1258291200 0 1258291201 0 2139095039 0 2139095040 0 2143289635 0 2139095041 0 2147483648 0 2147483649 0 2155872254 0 2155872255 0 2155872256 0 2155872257 0 3196059648 0 3204448255 0 3204448256 0 3204448257 0 3212836863 0 3212836864 0 3212836865 0 3217031167 0 3217031168 0 3217031169 0 3221225471 0 3221225472 0 3221225473 0 3223322623 0 3223322624 0 3223322625 0 3397386239 0 3397386240 0 3397386241 0 3405774847 0 3405774848 0 3405774849 0 4286578687 0 4286578688 0 4290773283 0 4286578689 0], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 1065353216 0 0 0 0 0 0 0 0 1065353216 0 0 0 0 0 0 0 0 1065353216 0 0 0 0 0 0 0 0 1065353216 0 0 0 0 0 0 0 0 1065353216 0 0 0 0 0 0 0 0 1065353216 0 0 0 0 0 0 0 0 1065353216 0 0 0 0 0 0 0 0 1065353216 0 0 0 0 0 0 0 0 1065353216 0 0 0 0 0 0 0 0 1065353216 0 0 0 0 0 0 0 0 1065353216 0 0 1065353216 0 0 1065353216 0 0 1073741824 0 0 1065353216 0 0 1065353216 0 0 1073741824 0 0 1065353216 0 0 1065353216 0 0 1073741824 0 0 1065353216 0 0 1065353216 0 0 1073741824 0 0 1065353216 0 0 1065353216 0 0 1073741824 0 0 1065353216 0 0 1065353216 0 0 1073741824 0 0 1073741824 0 0 1073741824 0 0 1077936128 0 0 1073741824 0 0 1073741824 0 0 1077936128 0 0 1073741824 0 0 1073741824 0 0 1077936128 0 0 1073741824 0 0 1073741824 0 0 1077936128 0 0 1073741824 0 0 1073741824 0 0 1249902592 0 0 1249902588 0 0 1249902588 0 0 1249902592 0 0 1249902592 0 0 1249902592 0 0 1249902594 0 0 1249902592 0 0 1249902592 0 0 1258291200 0 0 1258291198 0 0 1258291198 0 0 1258291200 0 0 1258291200 0 0 1258291200 0 0 1258291201 0 0 1258291201 0 0 1258291201 0 0 2139095039 0 0 2139095039 0 0 2139095039 0 0 2139095040 0 0 2139095040 0 0 2139095040 0 0 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 2147483648 0 0 2147483648 0 0 2147483648 0 0 2147483648 0 0 3212836864 0 0 2147483648 0 0 2147483648 0 0 3212836864 0 0 2147483648 0 0 2147483648 0 0 3212836864 0 0 2147483648 0 0 2147483648 0 0 3212836864 0 0 2147483648 0 0 2147483648 0 0 3212836864 0 0 2147483648 0 0 2147483648 0 0 3212836864 0 0 2147483648 0 0 2147483648 0 0 3212836864 0 0 2147483648 0 0 2147483648 0 0 3212836864 0 0 2147483648 0 0 2147483648 0 0 3212836864 0 0 2147483648 0 0 2147483648 0 0 3212836864 0 0 2147483648 0 0 3212836864 0 0 3212836864 0 0 3212836864 0 0 3212836864 0 0 3221225472 0 0 3212836864 0 0 3212836864 0 0 3221225472 0 0 3212836864 0 0 3212836864 0 0 3221225472 0 0 3212836864 0 0 3212836864 0 0 3221225472 0 0 3212836864 0 0 3212836864 0 0 3221225472 0 0 3212836864 0 0 3221225472 0 0 3221225472 0 0 3221225472 0 0 3221225472 0 0 3225419776 0 0 3221225472 0 0 3221225472 0 0 3225419776 0 0 3221225472 0 0 3221225472 0 0 3225419776 0 0 3221225472 0 0 3221225472 0 0 3225419776 0 0 3221225472 0 0 3397386236 0 0 3397386240 0 0 3397386236 0 0 3397386240 0 0 3397386240 0 0 3397386240 0 0 3397386240 0 0 3397386242 0 0 3397386240 0 0 3405774846 0 0 3405774848 0 0 3405774846 0 0 3405774848 0 0 3405774848 0 0 3405774848 0 0 3405774849 0 0 3405774849 0 0 3405774849 0 0 4286578687 0 0 4286578687 0 0 4286578687 0 0 4286578688 0 0 4286578688 0 0 4286578688 0 0 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

float loadValue(uint lane)
{
    return asfloat(inputWords[2 * lane]);
}

bool matchesExpected(float value, uint lane, uint operation)
{
    uint low = asuint(value);
    uint high = 0;
    bool nanBits = (low & 0x7f800000u) == 0x7f800000u && (low & 0x007fffffu) != 0;
    uint offset = 9 * lane + 3 * operation;
    return expectedWords[offset + 2] != 0 ? nanBits :
        low == expectedWords[offset] && high == expectedWords[offset + 1];
}

[noinline]
float ceilThroughHelper(float value)
{
    return ceil(value);
}

[noinline]
float floorThroughHelper(float value)
{
    return floor(value);
}

[noinline]
float truncThroughHelper(float value)
{
    return trunc(value);
}

[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 64) return;
    uint errors = 0;
    errors |= matchesExpected(ceil(loadValue(lane)), lane, 0) ? 0u : 1u;
    errors |= matchesExpected(ceilThroughHelper(loadValue(lane)), lane, 0) ? 0u : 2u;
    float2 ceilResult2 = ceil(float2(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64)));
    errors |= matchesExpected(ceilResult2.x, (lane + 0) % 64, 0) ? 0u : 4u;
    errors |= matchesExpected(ceilResult2.y, (lane + 1) % 64, 0) ? 0u : 4u;
    float3 ceilResult3 = ceil(float3(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64)));
    errors |= matchesExpected(ceilResult3.x, (lane + 0) % 64, 0) ? 0u : 8u;
    errors |= matchesExpected(ceilResult3.y, (lane + 1) % 64, 0) ? 0u : 8u;
    errors |= matchesExpected(ceilResult3.z, (lane + 2) % 64, 0) ? 0u : 8u;
    float4 ceilResult4 = ceil(float4(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(ceilResult4.x, (lane + 0) % 64, 0) ? 0u : 16u;
    errors |= matchesExpected(ceilResult4.y, (lane + 1) % 64, 0) ? 0u : 16u;
    errors |= matchesExpected(ceilResult4.z, (lane + 2) % 64, 0) ? 0u : 16u;
    errors |= matchesExpected(ceilResult4.w, (lane + 3) % 64, 0) ? 0u : 16u;
    matrix<float, 2, 2> ceilMatrix = ceil(matrix<float, 2, 2>(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(ceilMatrix[0][0], (lane + 0) % 64, 0) ? 0u : 32u;
    errors |= matchesExpected(ceilMatrix[0][1], (lane + 1) % 64, 0) ? 0u : 32u;
    errors |= matchesExpected(ceilMatrix[1][0], (lane + 2) % 64, 0) ? 0u : 32u;
    errors |= matchesExpected(ceilMatrix[1][1], (lane + 3) % 64, 0) ? 0u : 32u;
    errors |= matchesExpected(floor(loadValue(lane)), lane, 1) ? 0u : 64u;
    errors |= matchesExpected(floorThroughHelper(loadValue(lane)), lane, 1) ? 0u : 128u;
    float2 floorResult2 = floor(float2(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64)));
    errors |= matchesExpected(floorResult2.x, (lane + 0) % 64, 1) ? 0u : 256u;
    errors |= matchesExpected(floorResult2.y, (lane + 1) % 64, 1) ? 0u : 256u;
    float3 floorResult3 = floor(float3(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64)));
    errors |= matchesExpected(floorResult3.x, (lane + 0) % 64, 1) ? 0u : 512u;
    errors |= matchesExpected(floorResult3.y, (lane + 1) % 64, 1) ? 0u : 512u;
    errors |= matchesExpected(floorResult3.z, (lane + 2) % 64, 1) ? 0u : 512u;
    float4 floorResult4 = floor(float4(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(floorResult4.x, (lane + 0) % 64, 1) ? 0u : 1024u;
    errors |= matchesExpected(floorResult4.y, (lane + 1) % 64, 1) ? 0u : 1024u;
    errors |= matchesExpected(floorResult4.z, (lane + 2) % 64, 1) ? 0u : 1024u;
    errors |= matchesExpected(floorResult4.w, (lane + 3) % 64, 1) ? 0u : 1024u;
    matrix<float, 2, 2> floorMatrix = floor(matrix<float, 2, 2>(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(floorMatrix[0][0], (lane + 0) % 64, 1) ? 0u : 2048u;
    errors |= matchesExpected(floorMatrix[0][1], (lane + 1) % 64, 1) ? 0u : 2048u;
    errors |= matchesExpected(floorMatrix[1][0], (lane + 2) % 64, 1) ? 0u : 2048u;
    errors |= matchesExpected(floorMatrix[1][1], (lane + 3) % 64, 1) ? 0u : 2048u;
    errors |= matchesExpected(trunc(loadValue(lane)), lane, 2) ? 0u : 4096u;
    errors |= matchesExpected(truncThroughHelper(loadValue(lane)), lane, 2) ? 0u : 8192u;
    float2 truncResult2 = trunc(float2(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64)));
    errors |= matchesExpected(truncResult2.x, (lane + 0) % 64, 2) ? 0u : 16384u;
    errors |= matchesExpected(truncResult2.y, (lane + 1) % 64, 2) ? 0u : 16384u;
    float3 truncResult3 = trunc(float3(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64)));
    errors |= matchesExpected(truncResult3.x, (lane + 0) % 64, 2) ? 0u : 32768u;
    errors |= matchesExpected(truncResult3.y, (lane + 1) % 64, 2) ? 0u : 32768u;
    errors |= matchesExpected(truncResult3.z, (lane + 2) % 64, 2) ? 0u : 32768u;
    float4 truncResult4 = trunc(float4(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(truncResult4.x, (lane + 0) % 64, 2) ? 0u : 65536u;
    errors |= matchesExpected(truncResult4.y, (lane + 1) % 64, 2) ? 0u : 65536u;
    errors |= matchesExpected(truncResult4.z, (lane + 2) % 64, 2) ? 0u : 65536u;
    errors |= matchesExpected(truncResult4.w, (lane + 3) % 64, 2) ? 0u : 65536u;
    matrix<float, 2, 2> truncMatrix = trunc(matrix<float, 2, 2>(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(truncMatrix[0][0], (lane + 0) % 64, 2) ? 0u : 131072u;
    errors |= matchesExpected(truncMatrix[0][1], (lane + 1) % 64, 2) ? 0u : 131072u;
    errors |= matchesExpected(truncMatrix[1][0], (lane + 2) % 64, 2) ? 0u : 131072u;
    errors |= matchesExpected(truncMatrix[1][1], (lane + 3) % 64, 2) ? 0u : 131072u;
    outputBuffer[1 + 2 * lane] = errors;
    outputBuffer[2 + 2 * lane] = 16000 + lane;
}

// CHECK: 324508639{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16000{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16001{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16002{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16003{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16004{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16005{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16006{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16007{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16008{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16009{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16010{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16011{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16012{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16013{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16014{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16015{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16016{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16017{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16018{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16019{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16020{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16021{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16022{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16023{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16024{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16025{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16026{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16027{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16028{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16029{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16030{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16031{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16032{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16033{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16034{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16035{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16036{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16037{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16038{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16039{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16040{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16041{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16042{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16043{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16044{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16045{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16046{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16047{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16048{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16049{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16050{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16051{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16052{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16053{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16054{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16055{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16056{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16057{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16058{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16059{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16060{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16061{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16062{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 16063{{$}}
// CHECK-NEXT: 3735928559{{$}}
