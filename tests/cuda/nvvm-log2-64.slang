// Independent signed logarithm references are frozen before GPU observation.
// Library admission uses the empirical test-defined signed spacing/encoding union.
// NVVM Half narrows Float32 candidates once; CUDA Half checks documented RN-even
// on this finite corpus. CUDA double log10 preserves its existing Float32 wrapper.
// Every mode must also preserve its own complete baseline buffer byte for byte.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT: ubuffer(data=[0 0 1 0 2 0 4294967295 1048575 0 1048576 1 1048576 4294967295 1069547519 0 1069547520 1 1069547520 4294967295 1070596095 0 1070596096 1 1070596096 4294967295 1071644671 0 1071644672 1 1071644672 4294967295 1072168959 0 1072168960 1 1072168960 4294967295 1072693247 0 1072693248 1 1072693248 4294967295 1073217535 0 1073217536 1 1073217536 4294967295 1073741823 0 1073741824 1 1073741824 4294967295 1074266111 0 1074266112 1 1074266112 4294967295 1074790399 0 1074790400 1 1074790400 4294967295 1075838975 0 1075838976 1 1075838976 4294967295 1076101119 0 1076101120 1 1076101120 4294967295 1079574527 0 1079574528 1 1079574528 4294967295 1083129855 0 1083129856 1 1083129856 4294967295 2146435071 0 2146435072 1 2146435072 0 2146959360 0 2147483648 4294967295 3219128319 0 3219128320 1 3219128320 4294967295 3220176895 0 3220176896 1 3220176896 4294967295 3221225471 0 3221225472 1 3221225472 0 4293918720 1 4293918720 0 4294443008], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 1 0 4293918720 0 0 0 0 0 0 0 0 0 0 0 0 0 3 4294967295 3230713855 0 3230713856 1 3230713856 0 0 0 0 0 0 0 0 0 3 4294967295 3230712831 0 3230712832 1 3230712832 0 0 0 0 0 0 0 0 0 3 4294967295 3230658559 0 3230658560 1 3230658560 0 0 0 0 0 0 0 0 0 3 4294967295 3230658559 0 3230658560 1 3230658560 0 0 0 0 0 0 0 0 0 3 4294967295 3230658559 0 3230658560 1 3230658560 0 0 0 0 0 0 0 0 0 3 4294967295 3221749759 0 3221749760 1 3221749760 0 0 0 0 0 0 0 0 0 3 4294967295 3221749759 0 3221749760 1 3221749760 0 0 0 0 0 0 0 0 0 3 4294967294 3221749759 4294967295 3221749759 0 3221749760 0 0 0 0 0 0 0 0 0 4 4294967294 3221225471 4294967295 3221225471 0 3221225472 1 3221225472 0 0 0 0 0 0 0 4 4294967294 3221225471 4294967295 3221225471 0 3221225472 1 3221225472 0 0 0 0 0 0 0 3 4294967294 3221225471 4294967295 3221225471 0 3221225472 0 0 0 0 0 0 0 0 0 3 0 3220176896 1 3220176896 2 3220176896 0 0 0 0 0 0 0 0 0 4 4294967294 3220176895 4294967295 3220176895 0 3220176896 1 3220176896 0 0 0 0 0 0 0 3 4294967292 3220176895 4294967293 3220176895 4294967294 3220176895 0 0 0 0 0 0 0 0 0 3 1904282209 3218771961 1904282210 3218771961 1904282211 3218771961 0 0 0 0 0 0 0 0 0 3 1904282205 3218771961 1904282206 3218771961 1904282207 3218771961 0 0 0 0 0 0 0 0 0 3 1904282201 3218771961 1904282202 3218771961 1904282203 3218771961 0 0 0 0 0 0 0 0 0 3 1697350397 3165066567 1697350398 3165066567 1697350399 3165066567 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 1697350396 1018631495 1697350397 1018631495 1697350398 1018631495 0 0 0 0 0 0 0 0 0 3 1195342542 1071822851 1195342543 1071822851 1195342544 1071822851 0 0 0 0 0 0 0 0 0 3 1195342544 1071822851 1195342545 1071822851 1195342546 1071822851 0 0 0 0 0 0 0 0 0 3 1195342546 1071822851 1195342547 1071822851 1195342548 1071822851 0 0 0 0 0 0 0 0 0 3 4294967294 1072693247 4294967295 1072693247 0 1072693248 0 0 0 0 0 0 0 0 0 4 4294967294 1072693247 4294967295 1072693247 0 1072693248 1 1072693248 0 0 0 0 0 0 0 3 0 1072693248 1 1072693248 2 1072693248 0 0 0 0 0 0 0 0 0 3 2745154919 1073306625 2745154920 1073306625 2745154921 1073306625 0 0 0 0 0 0 0 0 0 3 2745154919 1073306625 2745154920 1073306625 2745154921 1073306625 0 0 0 0 0 0 0 0 0 3 2745154920 1073306625 2745154921 1073306625 2745154922 1073306625 0 0 0 0 0 0 0 0 0 3 4294967294 1073741823 4294967295 1073741823 0 1073741824 0 0 0 0 0 0 0 0 0 4 4294967294 1073741823 4294967295 1073741823 0 1073741824 1 1073741824 0 0 0 0 0 0 0 3 0 1073741824 1 1073741824 2 1073741824 0 0 0 0 0 0 0 0 0 3 4294967295 1074266111 0 1074266112 1 1074266112 0 0 0 0 0 0 0 0 0 3 4294967295 1074266111 0 1074266112 1 1074266112 0 0 0 0 0 0 0 0 0 3 0 1074266112 1 1074266112 2 1074266112 0 0 0 0 0 0 0 0 0 3 158966640 1074434895 158966641 1074434895 158966642 1074434895 0 0 0 0 0 0 0 0 0 3 158966640 1074434895 158966641 1074434895 158966642 1074434895 0 0 0 0 0 0 0 0 0 3 158966641 1074434895 158966642 1074434895 158966643 1074434895 0 0 0 0 0 0 0 0 0 3 158966640 1075483471 158966641 1075483471 158966642 1075483471 0 0 0 0 0 0 0 0 0 3 158966640 1075483471 158966641 1075483471 158966642 1075483471 0 0 0 0 0 0 0 0 0 3 158966641 1075483471 158966642 1075483471 158966643 1075483471 0 0 0 0 0 0 0 0 0 3 1192966804 1076096635 1192966805 1076096635 1192966806 1076096635 0 0 0 0 0 0 0 0 0 3 1192966804 1076096635 1192966805 1076096635 1192966806 1076096635 0 0 0 0 0 0 0 0 0 3 1192966804 1076096635 1192966805 1076096635 1192966806 1076096635 0 0 0 0 0 0 0 0 0 4 4294967294 1083179007 4294967295 1083179007 0 1083179008 1 1083179008 0 0 0 0 0 0 0 1 0 2146435072 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 4293918720 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

double loadValue(uint lane)
{
    return asdouble(inputWords[2 * lane], inputWords[2 * lane + 1]);
}

void valueBits(double value, out uint low, out uint high)
{
    asuint(value, low, high);
}

uint expectedWord(uint offset)
{
    return expectedWords[offset];
}

bool matchesExpected(double value, uint lane)
{
    uint low, high;
    valueBits(value, low, high);
    bool nanBits = (high & 0x7ff00000u) == 0x7ff00000u && ((high & 0xfffffu) != 0 || low != 0);
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
double log2ThroughHelper(double value)
{
    return log2(value);
}

[numthreads(128, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 62) return;
    uint errors = 0;
    double scalarResult = log2(loadValue(lane));
    errors |= matchesExpected(scalarResult, lane) ? 0u : 1u;
    errors |= matchesExpected(log2ThroughHelper(loadValue(lane)), lane) ? 0u : 2u;
    double2 result2 = log2(double2(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62)));
    errors |= matchesExpected(result2.x, (lane + 0) % 62) ? 0u : 4u;
    errors |= matchesExpected(result2.y, (lane + 1) % 62) ? 0u : 4u;
    double3 result3 = log2(double3(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62), loadValue((lane + 2) % 62)));
    errors |= matchesExpected(result3.x, (lane + 0) % 62) ? 0u : 8u;
    errors |= matchesExpected(result3.y, (lane + 1) % 62) ? 0u : 8u;
    errors |= matchesExpected(result3.z, (lane + 2) % 62) ? 0u : 8u;
    double4 result4 = log2(double4(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62), loadValue((lane + 2) % 62), loadValue((lane + 3) % 62)));
    errors |= matchesExpected(result4.x, (lane + 0) % 62) ? 0u : 16u;
    errors |= matchesExpected(result4.y, (lane + 1) % 62) ? 0u : 16u;
    errors |= matchesExpected(result4.z, (lane + 2) % 62) ? 0u : 16u;
    errors |= matchesExpected(result4.w, (lane + 3) % 62) ? 0u : 16u;
    matrix<double, 2, 2> resultMatrix = log2(matrix<double, 2, 2>(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62), loadValue((lane + 2) % 62), loadValue((lane + 3) % 62)));
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
