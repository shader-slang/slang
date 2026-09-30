// Exact rational frac oracle: RN-even(x - floor(x)); finite bits include +0.
// NaNs and either infinity require NaN classification, not payload equality.
// Half keeps the whole Float32 expression; its finite result equals direct Half RN.
// 64 live lanes; 26 observations/lane across every public frac/fract shape.
// Eleven shape-error bits, two raw scalar results, completion, and endpoint guards.
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT: ubuffer(data=[0 0 1 0 2 0 4294967295 1048575 0 1048576 1 1048576 4294967295 1016070143 0 1016070144 1 1016070144 4294967295 1071644671 0 1071644672 1 1071644672 4294967295 1072693247 0 1072693248 1 1072693248 4294967295 1073741823 0 1073741824 1 1073741824 4294967295 1074266111 0 1074266112 1 1074266112 4294967295 1127219199 0 1127219200 1 1127219200 4294967295 1128267775 0 1128267776 1 1128267776 0 1017118720 4294967295 2146435071 0 2146435072 35 2146959360 1 2146435072 0 2147483648 1 2147483648 2 2147483648 4294967295 2148532223 0 2148532224 1 2148532224 4294967295 3163553791 0 3163553792 1 3163553792 4294967295 3219128319 0 3219128320 1 3219128320 4294967295 3220176895 0 3220176896 1 3220176896 4294967295 3221225471 0 3221225472 1 3221225472 4294967295 3221749759 0 3221749760 1 3221749760 4294967295 3274702847 0 3274702848 1 3274702848 4294967295 3275751423 0 3275751424 1 3275751424 0 3164602368 4294967295 4293918719 0 4293918720 35 4294443008 1 4293918720], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 0 0 1 0 0 2 0 0 4294967295 1048575 0 0 1048576 0 1 1048576 0 4294967295 1016070143 0 0 1016070144 0 1 1016070144 0 4294967295 1071644671 0 0 1071644672 0 1 1071644672 0 4294967295 1072693247 0 0 0 0 0 1018167296 0 4294967294 1072693247 0 0 0 0 0 1019215872 0 4294967292 1072693247 0 0 0 0 0 1019215872 0 0 1071644672 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1017118720 0 0 0 0 0 0 1 0 0 1 0 0 1 0 0 0 0 1072693248 0 0 1072693248 0 0 1072693248 0 0 1072693248 0 0 1072693248 0 0 1072693248 0 0 1072693248 0 4294967295 1072693247 0 0 1071644672 0 0 1071644672 0 4294967294 1071644671 0 0 1017118720 0 0 0 0 4294967294 1072693247 0 0 1018167296 0 0 0 0 4294967292 1072693247 0 0 1019215872 0 0 0 0 4294967292 1072693247 0 0 1071644672 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 4294967295 1072693247 0 0 0 0 0 0 1 0 0 1 0 0 1], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

double loadValue(uint lane)
{
    return asdouble(inputWords[2 * lane], inputWords[2 * lane + 1]);
}

void valueBits(double value, out uint low, out uint high)
{
    asuint(value, low, high);
}

bool matchesExpected(double value, uint lane)
{
    uint low, high;
    valueBits(value, low, high);
    bool nanBits = (high & 0x7ff00000u) == 0x7ff00000u && ((high & 0xfffffu) != 0 || low != 0);
    uint offset = 3 * lane;
    return expectedWords[offset + 2] != 0 ? nanBits :
        low == expectedWords[offset] && high == expectedWords[offset + 1];
}

[noinline]
double fracThroughHelper(double value)
{
    return frac(value);
}

[noinline]
double fractThroughHelper(double value)
{
    return fract(value);
}

[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 64) return;
    uint errors = 0;
    double fracScalar = frac(loadValue(lane));
    errors |= matchesExpected(fracScalar, lane) ? 0u : 1u;
    errors |= matchesExpected(fracThroughHelper(loadValue(lane)), lane) ? 0u : 2u;
    double2 fracResult2 = frac(double2(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64)));
    errors |= matchesExpected(fracResult2.x, (lane + 0) % 64) ? 0u : 4u;
    errors |= matchesExpected(fracResult2.y, (lane + 1) % 64) ? 0u : 4u;
    double3 fracResult3 = frac(double3(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64)));
    errors |= matchesExpected(fracResult3.x, (lane + 0) % 64) ? 0u : 8u;
    errors |= matchesExpected(fracResult3.y, (lane + 1) % 64) ? 0u : 8u;
    errors |= matchesExpected(fracResult3.z, (lane + 2) % 64) ? 0u : 8u;
    double4 fracResult4 = frac(double4(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(fracResult4.x, (lane + 0) % 64) ? 0u : 16u;
    errors |= matchesExpected(fracResult4.y, (lane + 1) % 64) ? 0u : 16u;
    errors |= matchesExpected(fracResult4.z, (lane + 2) % 64) ? 0u : 16u;
    errors |= matchesExpected(fracResult4.w, (lane + 3) % 64) ? 0u : 16u;
    matrix<double, 2, 2> resultMatrix = frac(matrix<double, 2, 2>(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(resultMatrix[0][0], (lane + 0) % 64) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[0][1], (lane + 1) % 64) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][0], (lane + 2) % 64) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][1], (lane + 3) % 64) ? 0u : 32u;
    double fractScalar = fract(loadValue(lane));
    errors |= matchesExpected(fractScalar, lane) ? 0u : 64u;
    errors |= matchesExpected(fractThroughHelper(loadValue(lane)), lane) ? 0u : 128u;
    double2 fractResult2 = fract(double2(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64)));
    errors |= matchesExpected(fractResult2.x, (lane + 0) % 64) ? 0u : 256u;
    errors |= matchesExpected(fractResult2.y, (lane + 1) % 64) ? 0u : 256u;
    double3 fractResult3 = fract(double3(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64)));
    errors |= matchesExpected(fractResult3.x, (lane + 0) % 64) ? 0u : 512u;
    errors |= matchesExpected(fractResult3.y, (lane + 1) % 64) ? 0u : 512u;
    errors |= matchesExpected(fractResult3.z, (lane + 2) % 64) ? 0u : 512u;
    double4 fractResult4 = fract(double4(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
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
// CHECK-NEXT: 4294967295{{$}}
// CHECK-NEXT: 1048575{{$}}
// CHECK-NEXT: 4294967295{{$}}
// CHECK-NEXT: 1048575{{$}}
// CHECK-NEXT: 59003{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1048576{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1048576{{$}}
// CHECK-NEXT: 59004{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 1048576{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 1048576{{$}}
// CHECK-NEXT: 59005{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 4294967295{{$}}
// CHECK-NEXT: 1016070143{{$}}
// CHECK-NEXT: 4294967295{{$}}
// CHECK-NEXT: 1016070143{{$}}
// CHECK-NEXT: 59006{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1016070144{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1016070144{{$}}
// CHECK-NEXT: 59007{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 1016070144{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 1016070144{{$}}
// CHECK-NEXT: 59008{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 4294967295{{$}}
// CHECK-NEXT: 1071644671{{$}}
// CHECK-NEXT: 4294967295{{$}}
// CHECK-NEXT: 1071644671{{$}}
// CHECK-NEXT: 59009{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1071644672{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1071644672{{$}}
// CHECK-NEXT: 59010{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 1071644672{{$}}
// CHECK-NEXT: 1{{$}}
// CHECK-NEXT: 1071644672{{$}}
// CHECK-NEXT: 59011{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 4294967295{{$}}
// CHECK-NEXT: 1072693247{{$}}
// CHECK-NEXT: 4294967295{{$}}
// CHECK-NEXT: 1072693247{{$}}
// CHECK-NEXT: 59012{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59013{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1018167296{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1018167296{{$}}
// CHECK-NEXT: 59014{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 4294967294{{$}}
// CHECK-NEXT: 1072693247{{$}}
// CHECK-NEXT: 4294967294{{$}}
// CHECK-NEXT: 1072693247{{$}}
// CHECK-NEXT: 59015{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59016{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1019215872{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1019215872{{$}}
// CHECK-NEXT: 59017{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 4294967292{{$}}
// CHECK-NEXT: 1072693247{{$}}
// CHECK-NEXT: 4294967292{{$}}
// CHECK-NEXT: 1072693247{{$}}
// CHECK-NEXT: 59018{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59019{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1019215872{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1019215872{{$}}
// CHECK-NEXT: 59020{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1071644672{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1071644672{{$}}
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
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1017118720{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1017118720{{$}}
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
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1072693248{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1072693248{{$}}
// CHECK-NEXT: 59033{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1072693248{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1072693248{{$}}
// CHECK-NEXT: 59034{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1072693248{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1072693248{{$}}
// CHECK-NEXT: 59035{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1072693248{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1072693248{{$}}
// CHECK-NEXT: 59036{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1072693248{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1072693248{{$}}
// CHECK-NEXT: 59037{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1072693248{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1072693248{{$}}
// CHECK-NEXT: 59038{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1072693248{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1072693248{{$}}
// CHECK-NEXT: 59039{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 4294967295{{$}}
// CHECK-NEXT: 1072693247{{$}}
// CHECK-NEXT: 4294967295{{$}}
// CHECK-NEXT: 1072693247{{$}}
// CHECK-NEXT: 59040{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1071644672{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1071644672{{$}}
// CHECK-NEXT: 59041{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1071644672{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1071644672{{$}}
// CHECK-NEXT: 59042{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 4294967294{{$}}
// CHECK-NEXT: 1071644671{{$}}
// CHECK-NEXT: 4294967294{{$}}
// CHECK-NEXT: 1071644671{{$}}
// CHECK-NEXT: 59043{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1017118720{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1017118720{{$}}
// CHECK-NEXT: 59044{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59045{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 4294967294{{$}}
// CHECK-NEXT: 1072693247{{$}}
// CHECK-NEXT: 4294967294{{$}}
// CHECK-NEXT: 1072693247{{$}}
// CHECK-NEXT: 59046{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1018167296{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1018167296{{$}}
// CHECK-NEXT: 59047{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59048{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 4294967292{{$}}
// CHECK-NEXT: 1072693247{{$}}
// CHECK-NEXT: 4294967292{{$}}
// CHECK-NEXT: 1072693247{{$}}
// CHECK-NEXT: 59049{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1019215872{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1019215872{{$}}
// CHECK-NEXT: 59050{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 59051{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 4294967292{{$}}
// CHECK-NEXT: 1072693247{{$}}
// CHECK-NEXT: 4294967292{{$}}
// CHECK-NEXT: 1072693247{{$}}
// CHECK-NEXT: 59052{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1071644672{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 1071644672{{$}}
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
// CHECK-NEXT: 4294967295{{$}}
// CHECK-NEXT: 1072693247{{$}}
// CHECK-NEXT: 4294967295{{$}}
// CHECK-NEXT: 1072693247{{$}}
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
