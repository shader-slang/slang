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

//TEST_INPUT: ubuffer(data=[0 1 4286578688 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 3268316879 0 3268316880 0 3268316881 0 0 0 0 0 0 0 0 0 0 3 3268226027 0 3268226028 0 3268226029 0 0 0 0 0 0 0 0 0 0 3 3266227279 0 3266227280 0 3266227281 0 0 0 0 0 0 0 0 0 0 3 3266227279 0 3266227280 0 3266227281 0 0 0 0 0 0 0 0 0 0 3 3266227279 0 3266227280 0 3266227281 0 0 0 0 0 0 0 0 0 0 3 3221558673 0 3221558674 0 3221558675 0 0 0 0 0 0 0 0 0 0 3 3221558673 0 3221558674 0 3221558675 0 0 0 0 0 0 0 0 0 0 3 3221558672 0 3221558673 0 3221558674 0 0 0 0 0 0 0 0 0 0 3 3216077335 0 3216077336 0 3216077337 0 0 0 0 0 0 0 0 0 0 3 3216077335 0 3216077336 0 3216077337 0 0 0 0 0 0 0 0 0 0 3 3216077334 0 3216077335 0 3216077336 0 0 0 0 0 0 0 0 0 0 3 3207688728 0 3207688729 0 3207688730 0 0 0 0 0 0 0 0 0 0 3 3207688727 0 3207688728 0 3207688729 0 0 0 0 0 0 0 0 0 0 3 3207688725 0 3207688726 0 3207688727 0 0 0 0 0 0 0 0 0 0 3 3197324050 0 3197324051 0 3197324052 0 0 0 0 0 0 0 0 0 0 3 3197324048 0 3197324049 0 3197324050 0 0 0 0 0 0 0 0 0 0 3 3197324045 0 3197324046 0 3197324047 0 0 0 0 0 0 0 0 0 0 4 3011510270 0 3011510271 0 3011510272 0 3011510273 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 872415230 0 872415231 0 872415232 0 0 0 0 0 0 0 0 0 0 3 1053792540 0 1053792541 0 1053792542 0 0 0 0 0 0 0 0 0 0 3 1053792542 0 1053792543 0 1053792544 0 0 0 0 0 0 0 0 0 0 3 1053792545 0 1053792546 0 1053792547 0 0 0 0 0 0 0 0 0 0 3 1060205078 0 1060205079 0 1060205080 0 0 0 0 0 0 0 0 0 0 3 1060205079 0 1060205080 0 1060205081 0 0 0 0 0 0 0 0 0 0 3 1060205081 0 1060205082 0 1060205083 0 0 0 0 0 0 0 0 0 0 3 1066180434 0 1066180435 0 1066180436 0 0 0 0 0 0 0 0 0 0 3 1066180435 0 1066180436 0 1066180437 0 0 0 0 0 0 0 0 0 0 3 1066180436 0 1066180437 0 1066180438 0 0 0 0 0 0 0 0 0 0 3 1068593686 0 1068593687 0 1068593688 0 0 0 0 0 0 0 0 0 0 3 1068593687 0 1068593688 0 1068593689 0 0 0 0 0 0 0 0 0 0 3 1068593688 0 1068593689 0 1068593690 0 0 0 0 0 0 0 0 0 0 3 1074075025 0 1074075026 0 1074075027 0 0 0 0 0 0 0 0 0 0 3 1074075025 0 1074075026 0 1074075027 0 0 0 0 0 0 0 0 0 0 3 1074075025 0 1074075026 0 1074075027 0 0 0 0 0 0 0 0 0 0 3 1075010956 0 1075010957 0 1075010958 0 0 0 0 0 0 0 0 0 0 3 1075010957 0 1075010958 0 1075010959 0 0 0 0 0 0 0 0 0 0 3 1075010957 0 1075010958 0 1075010959 0 0 0 0 0 0 0 0 0 0 3 1083399565 0 1083399566 0 1083399567 0 0 0 0 0 0 0 0 0 0 3 1083399565 0 1083399566 0 1083399567 0 0 0 0 0 0 0 0 0 0 3 1083399565 0 1083399566 0 1083399567 0 0 0 0 0 0 0 0 0 0 3 1088228436 0 1088228437 0 1088228438 0 0 0 0 0 0 0 0 0 0 3 1088228436 0 1088228437 0 1088228438 0 0 0 0 0 0 0 0 0 0 3 1088228436 0 1088228437 0 1088228438 0 0 0 0 0 0 0 0 0 0 3 1118925335 0 1118925336 0 1118925337 0 0 0 0 0 0 0 0 0 0 1 2139095040 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 4286578688 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):name=expectedWords
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
float logThroughHelper(float value)
{
    return log(value);
}

[numthreads(128, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 62) return;
    uint errors = 0;
    float scalarResult = log(loadValue(lane));
    errors |= matchesExpected(scalarResult, lane) ? 0u : 1u;
    errors |= matchesExpected(logThroughHelper(loadValue(lane)), lane) ? 0u : 2u;
    float2 result2 = log(float2(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62)));
    errors |= matchesExpected(result2.x, (lane + 0) % 62) ? 0u : 4u;
    errors |= matchesExpected(result2.y, (lane + 1) % 62) ? 0u : 4u;
    float3 result3 = log(float3(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62), loadValue((lane + 2) % 62)));
    errors |= matchesExpected(result3.x, (lane + 0) % 62) ? 0u : 8u;
    errors |= matchesExpected(result3.y, (lane + 1) % 62) ? 0u : 8u;
    errors |= matchesExpected(result3.z, (lane + 2) % 62) ? 0u : 8u;
    float4 result4 = log(float4(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62), loadValue((lane + 2) % 62), loadValue((lane + 3) % 62)));
    errors |= matchesExpected(result4.x, (lane + 0) % 62) ? 0u : 16u;
    errors |= matchesExpected(result4.y, (lane + 1) % 62) ? 0u : 16u;
    errors |= matchesExpected(result4.z, (lane + 2) % 62) ? 0u : 16u;
    errors |= matchesExpected(result4.w, (lane + 3) % 62) ? 0u : 16u;
    matrix<float, 2, 2> resultMatrix = log(matrix<float, 2, 2>(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62), loadValue((lane + 2) % 62), loadValue((lane + 3) % 62)));
    errors |= matchesExpected(resultMatrix[0][0], (lane + 0) % 62) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[0][1], (lane + 1) % 62) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][0], (lane + 2) % 62) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][1], (lane + 3) % 62) ? 0u : 32u;
    uint low, high;
    valueBits(scalarResult, low, high);
    outputBuffer[1 + 4 * lane] = errors;
    outputBuffer[2 + 4 * lane] = low;
    outputBuffer[3 + 4 * lane] = high;
    outputBuffer[4 + 4 * lane] = 60000 + lane;
}

// CHECK: 324508639{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60000{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60001{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60002{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60003{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60004{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60005{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60006{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60007{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60008{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60009{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60010{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60011{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60012{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60013{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60014{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60015{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60016{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60017{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60018{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60019{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60020{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60021{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60022{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60023{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60024{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60025{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60026{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60027{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60028{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60029{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60030{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60031{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60032{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60033{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60034{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60035{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60036{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60037{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60038{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60039{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60040{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60041{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60042{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60043{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60044{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60045{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60046{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60047{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60048{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60049{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60050{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60051{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60052{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60053{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60054{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60055{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60056{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60057{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60058{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60059{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60060{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 60061{{$}}
// CHECK-NEXT: 3735928559{{$}}
