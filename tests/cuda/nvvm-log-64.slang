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

//TEST_INPUT: ubuffer(data=[0 1 0 4293918720 0 0 0 0 0 0 0 0 0 0 0 0 0 3 1148023234 3230090117 1148023235 3230090117 1148023236 3230090117 0 0 0 0 0 0 0 0 0 3 3014505268 3230088697 3014505269 3230088697 3014505270 3230088697 0 0 0 0 0 0 0 0 0 3 3715808465 3230016299 3715808466 3230016299 3715808467 3230016299 0 0 0 0 0 0 0 0 0 3 3715808465 3230016299 3715808466 3230016299 3715808467 3230016299 0 0 0 0 0 0 0 0 0 3 3715808465 3230016299 3715808466 3230016299 3715808467 3230016299 0 0 0 0 0 0 0 0 0 3 1060875123 3221267122 1060875124 3221267122 1060875125 3221267122 0 0 0 0 0 0 0 0 0 3 1060875122 3221267122 1060875123 3221267122 1060875124 3221267122 0 0 0 0 0 0 0 0 0 3 1060875122 3221267122 1060875123 3221267122 1060875124 3221267122 0 0 0 0 0 0 0 0 0 3 4277811695 3220581954 4277811696 3220581954 4277811697 3220581954 0 0 0 0 0 0 0 0 0 3 4277811694 3220581954 4277811695 3220581954 4277811696 3220581954 0 0 0 0 0 0 0 0 0 3 4277811693 3220581954 4277811694 3220581954 4277811695 3220581954 0 0 0 0 0 0 0 0 0 3 4277811695 3219533378 4277811696 3219533378 4277811697 3219533378 0 0 0 0 0 0 0 0 0 3 4277811694 3219533378 4277811695 3219533378 4277811696 3219533378 0 0 0 0 0 0 0 0 0 3 4277811692 3219533378 4277811693 3219533378 4277811694 3219533378 0 0 0 0 0 0 0 0 0 3 288676756 3218237794 288676757 3218237794 288676758 3218237794 0 0 0 0 0 0 0 0 0 3 288676753 3218237794 288676754 3218237794 288676755 3218237794 0 0 0 0 0 0 0 0 0 3 288676751 3218237794 288676752 3218237794 288676753 3218237794 0 0 0 0 0 0 0 0 0 4 4294967294 3164602367 4294967295 3164602367 0 3164602368 1 3164602368 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3 4294967294 1018167295 4294967295 1018167295 0 1018167296 0 0 0 0 0 0 0 0 0 3 3971979336 1071248163 3971979337 1071248163 3971979338 1071248163 0 0 0 0 0 0 0 0 0 3 3971979339 1071248163 3971979340 1071248163 3971979341 1071248163 0 0 0 0 0 0 0 0 0 3 3971979342 1071248163 3971979343 1071248163 3971979344 1071248163 0 0 0 0 0 0 0 0 0 3 4277811693 1072049730 4277811694 1072049730 4277811695 1072049730 0 0 0 0 0 0 0 0 0 3 4277811694 1072049730 4277811695 1072049730 4277811696 1072049730 0 0 0 0 0 0 0 0 0 3 4277811696 1072049730 4277811697 1072049730 4277811698 1072049730 0 0 0 0 0 0 0 0 0 3 2058158857 1072796650 2058158858 1072796650 2058158859 1072796650 0 0 0 0 0 0 0 0 0 3 2058158858 1072796650 2058158859 1072796650 2058158860 1072796650 0 0 0 0 0 0 0 0 0 3 2058158858 1072796650 2058158859 1072796650 2058158860 1072796650 0 0 0 0 0 0 0 0 0 3 4277811694 1073098306 4277811695 1073098306 4277811696 1073098306 0 0 0 0 0 0 0 0 0 3 4277811694 1073098306 4277811695 1073098306 4277811696 1073098306 0 0 0 0 0 0 0 0 0 3 4277811695 1073098306 4277811696 1073098306 4277811697 1073098306 0 0 0 0 0 0 0 0 0 3 1060875122 1073783474 1060875123 1073783474 1060875124 1073783474 0 0 0 0 0 0 0 0 0 3 1060875122 1073783474 1060875123 1073783474 1060875124 1073783474 0 0 0 0 0 0 0 0 0 3 1060875123 1073783474 1060875124 1073783474 1060875125 1073783474 0 0 0 0 0 0 0 0 0 3 3149223188 1073900465 3149223189 1073900465 3149223190 1073900465 0 0 0 0 0 0 0 0 0 3 3149223189 1073900465 3149223190 1073900465 3149223191 1073900465 0 0 0 0 0 0 0 0 0 3 3149223189 1073900465 3149223190 1073900465 3149223191 1073900465 0 0 0 0 0 0 0 0 0 3 3149223188 1074949041 3149223189 1074949041 3149223190 1074949041 0 0 0 0 0 0 0 0 0 3 3149223189 1074949041 3149223190 1074949041 3149223191 1074949041 0 0 0 0 0 0 0 0 0 3 3149223189 1074949041 3149223190 1074949041 3149223191 1074949041 0 0 0 0 0 0 0 0 0 3 2576351135 1075552650 2576351136 1075552650 2576351137 1075552650 0 0 0 0 0 0 0 0 0 3 2576351135 1075552650 2576351136 1075552650 2576351137 1075552650 0 0 0 0 0 0 0 0 0 3 2576351135 1075552650 2576351136 1075552650 2576351137 1075552650 0 0 0 0 0 0 0 0 0 3 4277811694 1082535490 4277811695 1082535490 4277811696 1082535490 0 0 0 0 0 0 0 0 0 1 0 2146435072 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 4293918720 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):name=expectedWords
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
double logThroughHelper(double value)
{
    return log(value);
}

[numthreads(128, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 62) return;
    uint errors = 0;
    double scalarResult = log(loadValue(lane));
    errors |= matchesExpected(scalarResult, lane) ? 0u : 1u;
    errors |= matchesExpected(logThroughHelper(loadValue(lane)), lane) ? 0u : 2u;
    double2 result2 = log(double2(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62)));
    errors |= matchesExpected(result2.x, (lane + 0) % 62) ? 0u : 4u;
    errors |= matchesExpected(result2.y, (lane + 1) % 62) ? 0u : 4u;
    double3 result3 = log(double3(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62), loadValue((lane + 2) % 62)));
    errors |= matchesExpected(result3.x, (lane + 0) % 62) ? 0u : 8u;
    errors |= matchesExpected(result3.y, (lane + 1) % 62) ? 0u : 8u;
    errors |= matchesExpected(result3.z, (lane + 2) % 62) ? 0u : 8u;
    double4 result4 = log(double4(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62), loadValue((lane + 2) % 62), loadValue((lane + 3) % 62)));
    errors |= matchesExpected(result4.x, (lane + 0) % 62) ? 0u : 16u;
    errors |= matchesExpected(result4.y, (lane + 1) % 62) ? 0u : 16u;
    errors |= matchesExpected(result4.z, (lane + 2) % 62) ? 0u : 16u;
    errors |= matchesExpected(result4.w, (lane + 3) % 62) ? 0u : 16u;
    matrix<double, 2, 2> resultMatrix = log(matrix<double, 2, 2>(loadValue((lane + 0) % 62), loadValue((lane + 1) % 62), loadValue((lane + 2) % 62), loadValue((lane + 3) % 62)));
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
