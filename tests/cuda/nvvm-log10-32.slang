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

//TEST_INPUT: ubuffer(data=[0 1 4286578688 0 0 0 0 0 0 0 0 0 0 0 0 0 0 5 3258149362 0 3258149363 0 3258149364 0 3258149365 0 3258149366 0 0 0 0 0 0 5 3258070449 0 3258070450 0 3258070451 0 3258070452 0 3258070453 0 0 0 0 0 0 5 3256334358 0 3256334359 0 3256334360 0 3256334361 0 3256334362 0 0 0 0 0 0 5 3256334358 0 3256334359 0 3256334360 0 3256334361 0 3256334362 0 0 0 0 0 0 5 3256334358 0 3256334359 0 3256334360 0 3256334361 0 3256334362 0 0 0 0 0 0 5 3211210982 0 3211210983 0 3211210984 0 3211210985 0 3211210986 0 0 0 0 0 0 5 3211210982 0 3211210983 0 3211210984 0 3211210985 0 3211210986 0 0 0 0 0 0 5 3211210981 0 3211210982 0 3211210983 0 3211210984 0 3211210985 0 0 0 0 0 0 5 3206160537 0 3206160538 0 3206160539 0 3206160540 0 3206160541 0 0 0 0 0 0 5 3206160537 0 3206160538 0 3206160539 0 3206160540 0 3206160541 0 0 0 0 0 0 5 3206160536 0 3206160537 0 3206160538 0 3206160539 0 3206160540 0 0 0 0 0 0 5 3197771929 0 3197771930 0 3197771931 0 3197771932 0 3197771933 0 0 0 0 0 0 5 3197771929 0 3197771930 0 3197771931 0 3197771932 0 3197771933 0 0 0 0 0 0 5 3197771927 0 3197771928 0 3197771929 0 3197771930 0 3197771931 0 0 0 0 0 0 5 3187662820 0 3187662821 0 3187662822 0 3187662823 0 3187662824 0 0 0 0 0 0 5 3187662815 0 3187662816 0 3187662817 0 3187662818 0 3187662819 0 0 0 0 0 0 5 3187662811 0 3187662812 0 3187662813 0 3187662814 0 3187662815 0 0 0 0 0 0 5 3000916951 0 3000916952 0 3000916953 0 3000916954 0 3000916955 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 5 861821910 0 861821911 0 861821912 0 861821913 0 861821914 0 0 0 0 0 0 5 1043616064 0 1043616065 0 1043616066 0 1043616067 0 1043616068 0 0 0 0 0 0 5 1043616066 0 1043616067 0 1043616068 0 1043616069 0 1043616070 0 0 0 0 0 0 5 1043616069 0 1043616070 0 1043616071 0 1043616072 0 1043616073 0 0 0 0 0 0 5 1050288280 0 1050288281 0 1050288282 0 1050288283 0 1050288284 0 0 0 0 0 0 5 1050288281 0 1050288282 0 1050288283 0 1050288284 0 1050288285 0 0 0 0 0 0 5 1050288282 0 1050288283 0 1050288284 0 1050288285 0 1050288286 0 0 0 0 0 0 5 1056196922 0 1056196923 0 1056196924 0 1056196925 0 1056196926 0 0 0 0 0 0 5 1056196923 0 1056196924 0 1056196925 0 1056196926 0 1056196927 0 0 0 0 0 0 5 1056196924 0 1056196925 0 1056196926 0 1056196927 0 1056196928 0 0 0 0 0 0 5 1058676888 0 1058676889 0 1058676890 0 1058676891 0 1058676892 0 0 0 0 0 0 5 1058676889 0 1058676890 0 1058676891 0 1058676892 0 1058676893 0 0 0 0 0 0 5 1058676889 0 1058676890 0 1058676891 0 1058676892 0 1058676893 0 0 0 0 0 0 5 1063727333 0 1063727334 0 1063727335 0 1063727336 0 1063727337 0 0 0 0 0 0 5 1063727334 0 1063727335 0 1063727336 0 1063727337 0 1063727338 0 0 0 0 0 0 5 1063727335 0 1063727336 0 1063727337 0 1063727338 0 1063727339 0 0 0 0 0 0 5 1065353213 0 1065353214 0 1065353215 0 1065353216 0 1065353217 0 0 0 0 0 0 7 1065353212 0 1065353213 0 1065353214 0 1065353215 0 1065353216 0 1065353217 0 1065353218 0 0 7 1065353212 0 1065353213 0 1065353214 0 1065353215 0 1065353216 0 1065353217 0 1065353218 0 0 7 1073741820 0 1073741821 0 1073741822 0 1073741823 0 1073741824 0 1073741825 0 1073741826 0 0 7 1073741820 0 1073741821 0 1073741822 0 1073741823 0 1073741824 0 1073741825 0 1073741826 0 0 7 1073741820 0 1073741821 0 1073741822 0 1073741823 0 1073741824 0 1073741825 0 1073741826 0 0 5 1077936126 0 1077936127 0 1077936128 0 1077936129 0 1077936130 0 0 0 0 0 0 5 1077936126 0 1077936127 0 1077936128 0 1077936129 0 1077936130 0 0 0 0 0 0 5 1077936126 0 1077936127 0 1077936128 0 1077936129 0 1077936130 0 0 0 0 0 0 5 1109008537 0 1109008538 0 1109008539 0 1109008540 0 1109008541 0 0 0 0 0 0 1 2139095040 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 4286578688 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):name=expectedWords
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
float log10ThroughHelper(float value)
{
    return log10(value);
}

[numthreads(128, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 62) return;
    uint errors = 0;
    float scalarResult = log10(loadValue(lane));
    errors |= matchesExpected(scalarResult, lane) ? 0u : 1u;
    errors |= matchesExpected(log10ThroughHelper(loadValue(lane)), lane) ? 0u : 2u;
    float2 result2 = log10(float2(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62)));
    errors |= matchesExpected(result2.x, (lane + 0) % 62) ? 0u : 4u;
    errors |= matchesExpected(result2.y, (lane + 1) % 62) ? 0u : 4u;
    float3 result3 = log10(float3(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62), loadValue((lane + 2) % 62)));
    errors |= matchesExpected(result3.x, (lane + 0) % 62) ? 0u : 8u;
    errors |= matchesExpected(result3.y, (lane + 1) % 62) ? 0u : 8u;
    errors |= matchesExpected(result3.z, (lane + 2) % 62) ? 0u : 8u;
    float4 result4 = log10(float4(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62), loadValue((lane + 2) % 62), loadValue((lane + 3) % 62)));
    errors |= matchesExpected(result4.x, (lane + 0) % 62) ? 0u : 16u;
    errors |= matchesExpected(result4.y, (lane + 1) % 62) ? 0u : 16u;
    errors |= matchesExpected(result4.z, (lane + 2) % 62) ? 0u : 16u;
    errors |= matchesExpected(result4.w, (lane + 3) % 62) ? 0u : 16u;
    matrix<float, 2, 2> resultMatrix = log10(matrix<float, 2, 2>(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62), loadValue((lane + 2) % 62), loadValue((lane + 3) % 62)));
    errors |= matchesExpected(resultMatrix[0][0], (lane + 0) % 62) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[0][1], (lane + 1) % 62) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][0], (lane + 2) % 62) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][1], (lane + 3) % 62) ? 0u : 32u;
    uint low, high;
    valueBits(scalarResult, low, high);
    outputBuffer[1 + 4 * lane] = errors;
    outputBuffer[2 + 4 * lane] = low;
    outputBuffer[3 + 4 * lane] = high;
    outputBuffer[4 + 4 * lane] = 62000 + lane;
}

// CHECK: 324508639{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62000{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62001{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62002{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62003{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62004{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62005{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62006{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62007{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62008{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62009{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62010{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62011{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62012{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62013{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62014{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62015{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62016{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62017{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62018{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62019{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62020{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62021{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62022{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62023{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62024{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62025{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62026{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62027{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62028{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62029{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62030{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62031{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62032{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62033{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62034{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62035{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62036{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62037{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62038{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62039{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62040{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62041{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62042{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62043{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62044{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62045{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62046{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62047{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62048{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62049{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62050{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62051{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62052{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62053{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62054{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62055{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62056{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62057{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62058{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62059{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62060{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 62061{{$}}
// CHECK-NEXT: 3735928559{{$}}
