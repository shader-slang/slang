// Independent signed logarithm references are frozen before GPU observation.
// Library admission uses the empirical test-defined signed spacing/encoding union.
// NVVM Half narrows Float32 candidates once; CUDA Half checks documented RN-even
// on this finite corpus. CUDA double log10 preserves its existing Float32 wrapper.
// Every mode must also preserve its own complete baseline buffer byte for byte.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT: ubuffer(data=[0 0 1 0 2 0 8388607 0 8388608 0 8388609 0 1040187391 0 1040187392 0 1040187393 0 1048575999 0 1048576000 0 1048576001 0 1056964607 0 1056964608 0 1056964609 0 1061158911 0 1061158912 0 1061158913 0 1065353215 0 1065353216 0 1065353217 0 1069547519 0 1069547520 0 1069547521 0 1073741823 0 1073741824 0 1073741825 0 1077936127 0 1077936128 0 1077936129 0 1082130431 0 1082130432 0 1082130433 0 1090519039 0 1090519040 0 1090519041 0 1092616191 0 1092616192 0 1092616193 0 1120403455 0 1120403456 0 1120403457 0 1148846079 0 1148846080 0 1148846081 0 2139095039 0 2139095040 0 2139095041 0 2143289344 0 2147483648 0 3204448255 0 3204448256 0 3204448257 0 3212836863 0 3212836864 0 3212836865 0 3221225471 0 3221225472 0 3221225473 0 4286578688 0 4286578689 0 4290772992 0], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 1 4286578688 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 3272933375 0 3272933376 0 3272933377 0 0 0 0 0 0 0 0 0 0 3 3272867839 0 3272867840 0 3272867841 0 0 0 0 0 0 0 0 0 0 3 3271294975 0 3271294976 0 3271294977 0 0 0 0 0 0 0 0 0 0 3 3271294975 0 3271294976 0 3271294977 0 0 0 0 0 0 0 0 0 0 3 3271294975 0 3271294976 0 3271294977 0 0 0 0 0 0 0 0 0 0 3 3225419775 0 3225419776 0 3225419777 0 0 0 0 0 0 0 0 0 0 3 3225419775 0 3225419776 0 3225419777 0 0 0 0 0 0 0 0 0 0 3 3225419774 0 3225419775 0 3225419776 0 0 0 0 0 0 0 0 0 0 4 3221225470 0 3221225471 0 3221225472 0 3221225473 0 0 0 0 0 0 0 0 4 3221225470 0 3221225471 0 3221225472 0 3221225473 0 0 0 0 0 0 0 0 3 3221225470 0 3221225471 0 3221225472 0 0 0 0 0 0 0 0 0 0 3 3212836864 0 3212836865 0 3212836866 0 0 0 0 0 0 0 0 0 0 4 3212836862 0 3212836863 0 3212836864 0 3212836865 0 0 0 0 0 0 0 0 3 3212836860 0 3212836861 0 3212836862 0 0 0 0 0 0 0 0 0 0 3 3201597390 0 3201597391 0 3201597392 0 0 0 0 0 0 0 0 0 0 3 3201597387 0 3201597388 0 3201597389 0 0 0 0 0 0 0 0 0 0 3 3201597383 0 3201597384 0 3201597385 0 0 0 0 0 0 0 0 0 0 3 3015223867 0 3015223868 0 3015223869 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 876128825 0 876128826 0 876128827 0 0 0 0 0 0 0 0 0 0 3 1058390039 0 1058390040 0 1058390041 0 0 0 0 0 0 0 0 0 0 3 1058390041 0 1058390042 0 1058390043 0 0 0 0 0 0 0 0 0 0 3 1058390043 0 1058390044 0 1058390045 0 0 0 0 0 0 0 0 0 0 3 1065353214 0 1065353215 0 1065353216 0 0 0 0 0 0 0 0 0 0 4 1065353214 0 1065353215 0 1065353216 0 1065353217 0 0 0 0 0 0 0 0 3 1065353216 0 1065353217 0 1065353218 0 0 0 0 0 0 0 0 0 0 3 1070260235 0 1070260236 0 1070260237 0 0 0 0 0 0 0 0 0 0 3 1070260236 0 1070260237 0 1070260238 0 0 0 0 0 0 0 0 0 0 3 1070260237 0 1070260238 0 1070260239 0 0 0 0 0 0 0 0 0 0 3 1073741822 0 1073741823 0 1073741824 0 0 0 0 0 0 0 0 0 0 4 1073741822 0 1073741823 0 1073741824 0 1073741825 0 0 0 0 0 0 0 0 3 1073741824 0 1073741825 0 1073741826 0 0 0 0 0 0 0 0 0 0 3 1077936127 0 1077936128 0 1077936129 0 0 0 0 0 0 0 0 0 0 3 1077936127 0 1077936128 0 1077936129 0 0 0 0 0 0 0 0 0 0 3 1077936128 0 1077936129 0 1077936130 0 0 0 0 0 0 0 0 0 0 3 1079286391 0 1079286392 0 1079286393 0 0 0 0 0 0 0 0 0 0 3 1079286391 0 1079286392 0 1079286393 0 0 0 0 0 0 0 0 0 0 3 1079286392 0 1079286393 0 1079286394 0 0 0 0 0 0 0 0 0 0 3 1087674999 0 1087675000 0 1087675001 0 0 0 0 0 0 0 0 0 0 3 1087674999 0 1087675000 0 1087675001 0 0 0 0 0 0 0 0 0 0 3 1087675000 0 1087675001 0 1087675002 0 0 0 0 0 0 0 0 0 0 3 1092580313 0 1092580314 0 1092580315 0 0 0 0 0 0 0 0 0 0 3 1092580313 0 1092580314 0 1092580315 0 0 0 0 0 0 0 0 0 0 3 1092580313 0 1092580314 0 1092580315 0 0 0 0 0 0 0 0 0 0 4 1124073470 0 1124073471 0 1124073472 0 1124073473 0 0 0 0 0 0 0 0 1 2139095040 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 4286578688 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
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

uint expectedWord(uint offset)
{
    return expectedWords[offset];
}

bool matchesExpected(float value, uint lane)
{
    uint low, high;
    valueBits(value, low, high);
    bool nanBits = (low & 0x7f800000u) == 0x7f800000u && (low & 0x7fffffu) != 0;
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
float log2ThroughHelper(float value)
{
    return log2(value);
}

[numthreads(128, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 62) return;
    uint errors = 0;
    float scalarResult = log2(loadValue(lane));
    errors |= matchesExpected(scalarResult, lane) ? 0u : 1u;
    errors |= matchesExpected(log2ThroughHelper(loadValue(lane)), lane) ? 0u : 2u;
    float2 result2 = log2(float2(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62)));
    errors |= matchesExpected(result2.x, (lane + 0) % 62) ? 0u : 4u;
    errors |= matchesExpected(result2.y, (lane + 1) % 62) ? 0u : 4u;
    float3 result3 = log2(float3(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62), loadValue((lane + 2) % 62)));
    errors |= matchesExpected(result3.x, (lane + 0) % 62) ? 0u : 8u;
    errors |= matchesExpected(result3.y, (lane + 1) % 62) ? 0u : 8u;
    errors |= matchesExpected(result3.z, (lane + 2) % 62) ? 0u : 8u;
    float4 result4 = log2(float4(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62), loadValue((lane + 2) % 62), loadValue((lane + 3) % 62)));
    errors |= matchesExpected(result4.x, (lane + 0) % 62) ? 0u : 16u;
    errors |= matchesExpected(result4.y, (lane + 1) % 62) ? 0u : 16u;
    errors |= matchesExpected(result4.z, (lane + 2) % 62) ? 0u : 16u;
    errors |= matchesExpected(result4.w, (lane + 3) % 62) ? 0u : 16u;
    matrix<float, 2, 2> resultMatrix = log2(matrix<float, 2, 2>(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62), loadValue((lane + 2) % 62), loadValue((lane + 3) % 62)));
    errors |= matchesExpected(resultMatrix[0][0], (lane + 0) % 62) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[0][1], (lane + 1) % 62) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][0], (lane + 2) % 62) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][1], (lane + 3) % 62) ? 0u : 32u;
    uint low, high;
    valueBits(scalarResult, low, high);
    outputBuffer[1 + 4 * lane] = errors;
    outputBuffer[2 + 4 * lane] = low;
    outputBuffer[3 + 4 * lane] = high;
    outputBuffer[4 + 4 * lane] = 61000 + lane;
}

// CHECK: 324508639{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61000{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61001{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61002{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61003{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61004{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61005{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61006{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61007{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61008{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61009{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61010{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61011{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61012{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61013{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61014{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61015{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61016{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61017{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61018{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61019{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61020{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61021{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61022{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61023{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61024{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61025{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61026{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61027{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61028{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61029{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61030{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61031{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61032{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61033{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61034{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61035{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61036{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61037{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61038{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61039{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61040{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61041{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61042{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61043{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61044{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61045{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61046{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61047{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61048{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61049{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61050{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61051{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61052{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61053{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61054{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61055{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61056{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61057{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61058{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61059{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61060{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 61061{{$}}
// CHECK-NEXT: 3735928559{{$}}
