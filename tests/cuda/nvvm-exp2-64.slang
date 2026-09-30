// Exp2 admission uses an explicitly test-defined spacing/encoding union, not a vendor ULP metric.
// Library N=2 for Float32 and N=1 for Float64 is an empirical, non-guaranteed qualification.
// Zero uses minsubnormal spacing; maxfinite uses the conceptual next binade; infinity uses
// encoding steps only. Special inputs have exact classification rules overriding that union.
// NVVM Half narrows the selected Float32 library set once. CUDA Half follows the installed
// ex2.approx.ftz/FMA.rn32(q,2^-24,q)/narrow sequence and has its own discrete candidate set.
// References are certified from exact input bits before observation; NaN payloads remain open.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT: ubuffer(data=[0 0 1 0 2 0 4294967295 1048575 0 1048576 1 1048576 796481474 1061622742 796481475 1061622742 796481476 1061622742 796481477 1061622742 1566281298 1063341239 1566281299 1063341239 1566281300 1063341239 1566281301 1063341239 0 1070596096 0 1071644672 4294967294 1072693247 4294967295 1072693247 0 1072693248 1 1072693248 4294967294 1073741823 4294967295 1073741823 0 1073741824 1 1073741824 0 1074790400 0 1075838976 0 1076887552 0 1083176960 4294967294 1083179007 4294967295 1083179007 0 1083179008 1 1083179008 4294967295 2146435071 0 2146435072 1 2146435072 0 2146959360 0 2147483648 1 2147483648 2 2147483648 4294967295 2148532223 0 2148532224 1 2148532224 0 3218079744 0 3219128320 4294967295 3220176895 0 3220176896 1 3220176896 2 3220176896 0 3221225472 0 3222274048 0 3223322624 0 3224371200 0 3225419776 0 3226468352 0 3227058176 0 3229999104 4294967295 3230658559 0 3230658560 1 3230658560 2 3230658560 0 3230713856 4294967295 3230714879 0 3230714880 1 3230714880 2 3230714880 4294967295 4293918719 0 4293918720 1 4293918720 0 4294443008], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 1 0 1072693248 0 0 0 0 0 0 0 0 0 0 0 0 0 4 4294967294 1072693247 4294967295 1072693247 0 1072693248 1 1072693248 0 0 0 0 0 0 0 4 4294967294 1072693247 4294967295 1072693247 0 1072693248 1 1072693248 0 0 0 0 0 0 0 4 4294967294 1072693247 4294967295 1072693247 0 1072693248 1 1072693248 0 0 0 0 0 0 0 4 4294967294 1072693247 4294967295 1072693247 0 1072693248 1 1072693248 0 0 0 0 0 0 0 4 4294967294 1072693247 4294967295 1072693247 0 1072693248 1 1072693248 0 0 0 0 0 0 0 3 4294967295 1072693759 0 1072693760 1 1072693760 0 0 0 0 0 0 0 0 0 3 4294967295 1072693759 0 1072693760 1 1072693760 0 0 0 0 0 0 0 0 0 3 4294967295 1072693759 0 1072693760 1 1072693760 0 0 0 0 0 0 0 0 0 3 4294967295 1072693759 0 1072693760 1 1072693760 0 0 0 0 0 0 0 0 0 3 4294967295 1072694783 0 1072694784 1 1072694784 0 0 0 0 0 0 0 0 0 3 4294967295 1072694783 0 1072694784 1 1072694784 0 0 0 0 0 0 0 0 0 3 4294967295 1072694783 0 1072694784 1 1072694784 0 0 0 0 0 0 0 0 0 3 4294967295 1072694783 0 1072694784 1 1072694784 0 0 0 0 0 0 0 0 0 3 171030292 1072891646 171030293 1072891646 171030294 1072891646 0 0 0 0 0 0 0 0 0 3 1719614412 1073127582 1719614413 1073127582 1719614414 1073127582 0 0 0 0 0 0 0 0 0 3 4294967294 1073741823 4294967295 1073741823 0 1073741824 0 0 0 0 0 0 0 0 0 3 4294967294 1073741823 4294967295 1073741823 0 1073741824 0 0 0 0 0 0 0 0 0 4 4294967294 1073741823 4294967295 1073741823 0 1073741824 1 1073741824 0 0 0 0 0 0 0 3 0 1073741824 1 1073741824 2 1073741824 0 0 0 0 0 0 0 0 0 3 4294967292 1074790399 4294967293 1074790399 4294967294 1074790399 0 0 0 0 0 0 0 0 0 3 4294967294 1074790399 4294967295 1074790399 0 1074790400 0 0 0 0 0 0 0 0 0 4 4294967294 1074790399 4294967295 1074790399 0 1074790400 1 1074790400 0 0 0 0 0 0 0 3 0 1074790400 1 1074790400 2 1074790400 0 0 0 0 0 0 0 0 0 4 4294967294 1076887551 4294967295 1076887551 0 1076887552 1 1076887552 0 0 0 0 0 0 0 4 4294967294 1081081855 4294967295 1081081855 0 1081081856 1 1081081856 0 0 0 0 0 0 0 4 4294967294 1089470463 4294967295 1089470463 0 1089470464 1 1089470464 0 0 0 0 0 0 0 4 4294967294 2145386495 4294967295 2145386495 0 2145386496 1 2145386496 0 0 0 0 0 0 0 3 4294965875 2146435071 4294965876 2146435071 4294965877 2146435071 0 0 0 0 0 0 0 0 0 3 4294966585 2146435071 4294966586 2146435071 4294966587 2146435071 0 0 0 0 0 0 0 0 0 2 4294967295 2146435071 0 2146435072 0 0 0 0 0 0 0 0 0 0 0 2 4294967295 2146435071 0 2146435072 0 0 0 0 0 0 0 0 0 0 0 2 4294967295 2146435071 0 2146435072 0 0 0 0 0 0 0 0 0 0 0 1 0 2146435072 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 1072693248 0 0 0 0 0 0 0 0 0 0 0 0 0 4 4294967294 1072693247 4294967295 1072693247 0 1072693248 1 1072693248 0 0 0 0 0 0 0 4 4294967294 1072693247 4294967295 1072693247 0 1072693248 1 1072693248 0 0 0 0 0 0 0 4 4294967294 1072693247 4294967295 1072693247 0 1072693248 1 1072693248 0 0 0 0 0 0 0 4 4294967294 1072693247 4294967295 1072693247 0 1072693248 1 1072693248 0 0 0 0 0 0 0 4 4294967294 1072693247 4294967295 1072693247 0 1072693248 1 1072693248 0 0 0 0 0 0 0 3 2572866476 1072359583 2572866477 1072359583 2572866478 1072359583 0 0 0 0 0 0 0 0 0 3 1719614412 1072079006 1719614413 1072079006 1719614414 1072079006 0 0 0 0 0 0 0 0 0 4 4294967294 1071644671 4294967295 1071644671 0 1071644672 1 1071644672 0 0 0 0 0 0 0 4 4294967294 1071644671 4294967295 1071644671 0 1071644672 1 1071644672 0 0 0 0 0 0 0 3 4294967294 1071644671 4294967295 1071644671 0 1071644672 0 0 0 0 0 0 0 0 0 3 4294967292 1071644671 4294967293 1071644671 4294967294 1071644671 0 0 0 0 0 0 0 0 0 4 4294967294 1070596095 4294967295 1070596095 0 1070596096 1 1070596096 0 0 0 0 0 0 0 4 4294967294 1068498943 4294967295 1068498943 0 1068498944 1 1068498944 0 0 0 0 0 0 0 4 4294967294 1064304639 4294967295 1064304639 0 1064304640 1 1064304640 0 0 0 0 0 0 0 4 4294967294 1055916031 4294967295 1055916031 0 1055916032 1 1055916032 0 0 0 0 0 0 0 4 4294967294 1039138815 4294967295 1039138815 0 1039138816 1 1039138816 0 0 0 0 0 0 0 4 4294967294 1005584383 4294967295 1005584383 0 1005584384 1 1005584384 0 0 0 0 0 0 0 4 4294967294 967835647 4294967295 967835647 0 967835648 1 967835648 0 0 0 0 0 0 0 4 4294967294 338690047 4294967295 338690047 0 338690048 1 338690048 0 0 0 0 0 0 0 3 354 1048576 355 1048576 356 1048576 0 0 0 0 0 0 0 0 0 3 4294967295 1048575 0 1048576 1 1048576 0 0 0 0 0 0 0 0 0 3 4294966940 1048575 4294966941 1048575 4294966942 1048575 0 0 0 0 0 0 0 0 0 3 4294966585 1048575 4294966586 1048575 4294966587 1048575 0 0 0 0 0 0 0 0 0 3 0 0 1 0 2 0 0 0 0 0 0 0 0 0 0 3 0 0 1 0 2 0 0 0 0 0 0 0 0 0 0 2 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 2 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 2 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 2 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
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
double exp2ThroughHelper(double value)
{
    return exp2(value);
}

[numthreads(128, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 69) return;
    uint errors = 0;
    double scalarResult = exp2(loadValue(lane));
    errors |= matchesExpected(scalarResult, lane) ? 0u : 1u;
    errors |= matchesExpected(exp2ThroughHelper(loadValue(lane)), lane) ? 0u : 2u;
    double2 result2 = exp2(double2(loadValue((lane + 0) % 69), loadValue((lane + 1) % 69)));
    errors |= matchesExpected(result2.x, (lane + 0) % 69) ? 0u : 4u;
    errors |= matchesExpected(result2.y, (lane + 1) % 69) ? 0u : 4u;
    double3 result3 = exp2(double3(loadValue((lane + 0) % 69), loadValue((lane + 1) % 69), loadValue((lane + 2) % 69)));
    errors |= matchesExpected(result3.x, (lane + 0) % 69) ? 0u : 8u;
    errors |= matchesExpected(result3.y, (lane + 1) % 69) ? 0u : 8u;
    errors |= matchesExpected(result3.z, (lane + 2) % 69) ? 0u : 8u;
    double4 result4 = exp2(double4(loadValue((lane + 0) % 69), loadValue((lane + 1) % 69), loadValue((lane + 2) % 69), loadValue((lane + 3) % 69)));
    errors |= matchesExpected(result4.x, (lane + 0) % 69) ? 0u : 16u;
    errors |= matchesExpected(result4.y, (lane + 1) % 69) ? 0u : 16u;
    errors |= matchesExpected(result4.z, (lane + 2) % 69) ? 0u : 16u;
    errors |= matchesExpected(result4.w, (lane + 3) % 69) ? 0u : 16u;
    matrix<double, 2, 2> resultMatrix = exp2(matrix<double, 2, 2>(loadValue((lane + 0) % 69), loadValue((lane + 1) % 69), loadValue((lane + 2) % 69), loadValue((lane + 3) % 69)));
    errors |= matchesExpected(resultMatrix[0][0], (lane + 0) % 69) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[0][1], (lane + 1) % 69) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][0], (lane + 2) % 69) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][1], (lane + 3) % 69) ? 0u : 32u;
    uint low, high;
    valueBits(scalarResult, low, high);
    outputBuffer[1 + 4 * lane] = errors;
    outputBuffer[2 + 4 * lane] = low;
    outputBuffer[3 + 4 * lane] = high;
    outputBuffer[4 + 4 * lane] = 56000 + lane;
}

// CHECK: 324508639{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56000{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56001{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56002{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56003{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56004{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56005{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56006{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56007{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56008{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56009{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56010{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56011{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56012{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56013{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56014{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56015{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56016{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56017{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56018{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56019{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56020{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56021{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56022{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56023{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56024{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56025{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56026{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56027{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56028{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56029{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56030{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56031{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56032{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56033{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56034{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56035{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56036{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56037{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56038{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56039{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56040{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56041{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56042{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56043{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56044{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56045{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56046{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56047{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56048{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56049{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56050{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56051{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56052{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56053{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56054{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56055{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56056{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56057{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56058{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56059{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56060{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56061{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56062{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56063{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56064{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56065{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56066{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56067{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: {{[0-9]+}}{{$}}
// CHECK-NEXT: 56068{{$}}
// CHECK-NEXT: 3735928559{{$}}
