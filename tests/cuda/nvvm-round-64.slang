// Frozen input/oracle constants generated independently of compiler execution.
// Rounding: nearest integer, ties away from zero; preserve signed zero and infinities.
// NaNs require only an IEEE NaN encoding, with no sign/payload promise.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT: ubuffer(data=[0 0 0 1070596096 4294967295 1071644671 0 1071644672 1 1071644672 4294967295 1073217535 0 1073217536 1 1073217536 4294967295 1074003967 0 1074003968 1 1074003968 0 1072693248 0 1073741824 1 0 4294967295 1048575 0 1048576 4294967295 1127219199 0 1127219200 1 1127219200 4294967295 1128267775 0 1128267776 1 1128267776 4294967295 2146435071 0 2146435072 291 2146959360 1 2146435072 0 2147483648 0 3218079744 4294967295 3219128319 0 3219128320 1 3219128320 4294967295 3220701183 0 3220701184 1 3220701184 4294967295 3221487615 0 3221487616 1 3221487616 0 3220176896 0 3221225472 1 2147483648 4294967295 2148532223 0 2148532224 4294967295 3274702847 0 3274702848 1 3274702848 4294967295 3275751423 0 3275751424 1 3275751424 4294967295 4293918719 0 4293918720 291 4294443008 1 4293918720], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 1072693248 0 0 1072693248 0 0 1072693248 0 0 1073741824 0 0 1073741824 0 0 1073741824 0 0 1074266112 0 0 1074266112 0 0 1072693248 0 0 1073741824 0 0 0 0 0 0 0 0 0 0 0 1127219200 0 0 1127219200 0 1 1127219200 0 4294967295 1128267775 0 0 1128267776 0 1 1128267776 0 4294967295 2146435071 0 0 2146435072 0 0 0 1 0 0 1 0 2147483648 0 0 2147483648 0 0 2147483648 0 0 3220176896 0 0 3220176896 0 0 3220176896 0 0 3221225472 0 0 3221225472 0 0 3221225472 0 0 3221749760 0 0 3221749760 0 0 3220176896 0 0 3221225472 0 0 2147483648 0 0 2147483648 0 0 2147483648 0 0 3274702848 0 0 3274702848 0 1 3274702848 0 4294967295 3275751423 0 0 3275751424 0 1 3275751424 0 4294967295 4293918719 0 0 4293918720 0 0 0 1 0 0 1], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

double loadValue(uint lane)
{
    return asdouble(inputWords[2 * lane], inputWords[2 * lane + 1]);
}

bool matchesExpected(double value, uint lane)
{
    uint low, high;
    asuint(value, low, high);
    bool nanBits = (high & 0x7ff00000u) == 0x7ff00000u &&
        ((high & 0x000fffffu) != 0 || low != 0);
    return expectedWords[3 * lane + 2] != 0 ? nanBits :
        low == expectedWords[3 * lane] && high == expectedWords[3 * lane + 1];
}

[noinline]
double roundThroughHelper(double value)
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
    double2 result2 = round(double2(loadValue((lane + 0) % 52), loadValue((lane + 1) % 52)));
    errors |= matchesExpected(result2.x, (lane + 0) % 52) ? 0u : 4u;
    errors |= matchesExpected(result2.y, (lane + 1) % 52) ? 0u : 4u;
    double3 result3 = round(double3(loadValue((lane + 0) % 52), loadValue((lane + 1) % 52), loadValue((lane + 2) % 52)));
    errors |= matchesExpected(result3.x, (lane + 0) % 52) ? 0u : 8u;
    errors |= matchesExpected(result3.y, (lane + 1) % 52) ? 0u : 8u;
    errors |= matchesExpected(result3.z, (lane + 2) % 52) ? 0u : 8u;
    double4 result4 = round(double4(loadValue((lane + 0) % 52), loadValue((lane + 1) % 52), loadValue((lane + 2) % 52), loadValue((lane + 3) % 52)));
    errors |= matchesExpected(result4.x, (lane + 0) % 52) ? 0u : 16u;
    errors |= matchesExpected(result4.y, (lane + 1) % 52) ? 0u : 16u;
    errors |= matchesExpected(result4.z, (lane + 2) % 52) ? 0u : 16u;
    errors |= matchesExpected(result4.w, (lane + 3) % 52) ? 0u : 16u;
    matrix<double, 2, 2> matrixResult = round(matrix<double, 2, 2>(loadValue((lane + 0) % 52), loadValue((lane + 1) % 52), loadValue((lane + 2) % 52), loadValue((lane + 3) % 52)));
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
