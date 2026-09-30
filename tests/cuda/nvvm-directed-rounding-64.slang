// Exact independent IEEE input/oracle constants; no target-dependent oracle selection.
// Ceil rounds toward +infinity, floor toward -infinity, trunc toward zero.
// Preserve signed zeros and infinities; compare NaNs by class only.
// Each output mask reserves six bits per operation: scalar, helper, vector2/3/4, matrix2x2.
// 64 live lanes exercise 45 results each; completion words and end guards are checked.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT: ubuffer(data=[0 0 1 0 4294967294 1048575 4294967295 1048575 0 1048576 1 1048576 0 1070596096 4294967295 1071644671 0 1071644672 1 1071644672 4294967295 1072693247 0 1072693248 1 1072693248 4294967295 1073217535 0 1073217536 1 1073217536 4294967295 1073741823 0 1073741824 1 1073741824 4294967295 1074003967 0 1074003968 1 1074003968 4294967295 1126170623 0 1126170624 1 1126170624 4294967295 1127219199 0 1127219200 1 1127219200 4294967295 2146435071 0 2146435072 291 2146959360 1 2146435072 0 2147483648 1 2147483648 4294967294 2148532223 4294967295 2148532223 0 2148532224 1 2148532224 0 3218079744 4294967295 3219128319 0 3219128320 1 3219128320 4294967295 3220176895 0 3220176896 1 3220176896 4294967295 3220701183 0 3220701184 1 3220701184 4294967295 3221225471 0 3221225472 1 3221225472 4294967295 3221487615 0 3221487616 1 3221487616 4294967295 3273654271 0 3273654272 1 3273654272 4294967295 3274702847 0 3274702848 1 3274702848 4294967295 4293918719 0 4293918720 291 4294443008 1 4293918720], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 0 0 0 0 0 0 0 0 0 1072693248 0 0 0 0 0 0 0 0 1072693248 0 0 0 0 0 0 0 0 1072693248 0 0 0 0 0 0 0 0 1072693248 0 0 0 0 0 0 0 0 1072693248 0 0 0 0 0 0 0 0 1072693248 0 0 0 0 0 0 0 0 1072693248 0 0 0 0 0 0 0 0 1072693248 0 0 0 0 0 0 0 0 1072693248 0 0 0 0 0 0 0 0 1072693248 0 0 0 0 0 0 0 0 1072693248 0 0 1072693248 0 0 1072693248 0 0 1073741824 0 0 1072693248 0 0 1072693248 0 0 1073741824 0 0 1072693248 0 0 1072693248 0 0 1073741824 0 0 1072693248 0 0 1072693248 0 0 1073741824 0 0 1072693248 0 0 1072693248 0 0 1073741824 0 0 1072693248 0 0 1072693248 0 0 1073741824 0 0 1073741824 0 0 1073741824 0 0 1074266112 0 0 1073741824 0 0 1073741824 0 0 1074266112 0 0 1073741824 0 0 1073741824 0 0 1074266112 0 0 1073741824 0 0 1073741824 0 0 1074266112 0 0 1073741824 0 0 1073741824 0 0 1126170624 0 4294967292 1126170623 0 4294967292 1126170623 0 0 1126170624 0 0 1126170624 0 0 1126170624 0 2 1126170624 0 0 1126170624 0 0 1126170624 0 0 1127219200 0 4294967294 1127219199 0 4294967294 1127219199 0 0 1127219200 0 0 1127219200 0 0 1127219200 0 1 1127219200 0 1 1127219200 0 1 1127219200 0 4294967295 2146435071 0 4294967295 2146435071 0 4294967295 2146435071 0 0 2146435072 0 0 2146435072 0 0 2146435072 0 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 2147483648 0 0 2147483648 0 0 2147483648 0 0 2147483648 0 0 3220176896 0 0 2147483648 0 0 2147483648 0 0 3220176896 0 0 2147483648 0 0 2147483648 0 0 3220176896 0 0 2147483648 0 0 2147483648 0 0 3220176896 0 0 2147483648 0 0 2147483648 0 0 3220176896 0 0 2147483648 0 0 2147483648 0 0 3220176896 0 0 2147483648 0 0 2147483648 0 0 3220176896 0 0 2147483648 0 0 2147483648 0 0 3220176896 0 0 2147483648 0 0 2147483648 0 0 3220176896 0 0 2147483648 0 0 2147483648 0 0 3220176896 0 0 2147483648 0 0 3220176896 0 0 3220176896 0 0 3220176896 0 0 3220176896 0 0 3221225472 0 0 3220176896 0 0 3220176896 0 0 3221225472 0 0 3220176896 0 0 3220176896 0 0 3221225472 0 0 3220176896 0 0 3220176896 0 0 3221225472 0 0 3220176896 0 0 3220176896 0 0 3221225472 0 0 3220176896 0 0 3221225472 0 0 3221225472 0 0 3221225472 0 0 3221225472 0 0 3221749760 0 0 3221225472 0 0 3221225472 0 0 3221749760 0 0 3221225472 0 0 3221225472 0 0 3221749760 0 0 3221225472 0 0 3221225472 0 0 3221749760 0 0 3221225472 0 4294967292 3273654271 0 0 3273654272 0 4294967292 3273654271 0 0 3273654272 0 0 3273654272 0 0 3273654272 0 0 3273654272 0 2 3273654272 0 0 3273654272 0 4294967294 3274702847 0 0 3274702848 0 4294967294 3274702847 0 0 3274702848 0 0 3274702848 0 0 3274702848 0 1 3274702848 0 1 3274702848 0 1 3274702848 0 4294967295 4293918719 0 4294967295 4293918719 0 4294967295 4293918719 0 0 4293918720 0 0 4293918720 0 0 4293918720 0 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

double loadValue(uint lane)
{
    return asdouble(inputWords[2 * lane], inputWords[2 * lane + 1]);
}

bool matchesExpected(double value, uint lane, uint operation)
{
    uint low, high;
    asuint(value, low, high);
    bool nanBits = (high & 0x7ff00000u) == 0x7ff00000u &&
        ((high & 0x000fffffu) != 0 || low != 0);
    uint offset = 9 * lane + 3 * operation;
    return expectedWords[offset + 2] != 0 ? nanBits :
        low == expectedWords[offset] && high == expectedWords[offset + 1];
}

[noinline]
double ceilThroughHelper(double value)
{
    return ceil(value);
}

[noinline]
double floorThroughHelper(double value)
{
    return floor(value);
}

[noinline]
double truncThroughHelper(double value)
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
    double2 ceilResult2 = ceil(double2(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64)));
    errors |= matchesExpected(ceilResult2.x, (lane + 0) % 64, 0) ? 0u : 4u;
    errors |= matchesExpected(ceilResult2.y, (lane + 1) % 64, 0) ? 0u : 4u;
    double3 ceilResult3 = ceil(double3(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64)));
    errors |= matchesExpected(ceilResult3.x, (lane + 0) % 64, 0) ? 0u : 8u;
    errors |= matchesExpected(ceilResult3.y, (lane + 1) % 64, 0) ? 0u : 8u;
    errors |= matchesExpected(ceilResult3.z, (lane + 2) % 64, 0) ? 0u : 8u;
    double4 ceilResult4 = ceil(double4(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(ceilResult4.x, (lane + 0) % 64, 0) ? 0u : 16u;
    errors |= matchesExpected(ceilResult4.y, (lane + 1) % 64, 0) ? 0u : 16u;
    errors |= matchesExpected(ceilResult4.z, (lane + 2) % 64, 0) ? 0u : 16u;
    errors |= matchesExpected(ceilResult4.w, (lane + 3) % 64, 0) ? 0u : 16u;
    matrix<double, 2, 2> ceilMatrix = ceil(matrix<double, 2, 2>(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(ceilMatrix[0][0], (lane + 0) % 64, 0) ? 0u : 32u;
    errors |= matchesExpected(ceilMatrix[0][1], (lane + 1) % 64, 0) ? 0u : 32u;
    errors |= matchesExpected(ceilMatrix[1][0], (lane + 2) % 64, 0) ? 0u : 32u;
    errors |= matchesExpected(ceilMatrix[1][1], (lane + 3) % 64, 0) ? 0u : 32u;
    errors |= matchesExpected(floor(loadValue(lane)), lane, 1) ? 0u : 64u;
    errors |= matchesExpected(floorThroughHelper(loadValue(lane)), lane, 1) ? 0u : 128u;
    double2 floorResult2 = floor(double2(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64)));
    errors |= matchesExpected(floorResult2.x, (lane + 0) % 64, 1) ? 0u : 256u;
    errors |= matchesExpected(floorResult2.y, (lane + 1) % 64, 1) ? 0u : 256u;
    double3 floorResult3 = floor(double3(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64)));
    errors |= matchesExpected(floorResult3.x, (lane + 0) % 64, 1) ? 0u : 512u;
    errors |= matchesExpected(floorResult3.y, (lane + 1) % 64, 1) ? 0u : 512u;
    errors |= matchesExpected(floorResult3.z, (lane + 2) % 64, 1) ? 0u : 512u;
    double4 floorResult4 = floor(double4(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(floorResult4.x, (lane + 0) % 64, 1) ? 0u : 1024u;
    errors |= matchesExpected(floorResult4.y, (lane + 1) % 64, 1) ? 0u : 1024u;
    errors |= matchesExpected(floorResult4.z, (lane + 2) % 64, 1) ? 0u : 1024u;
    errors |= matchesExpected(floorResult4.w, (lane + 3) % 64, 1) ? 0u : 1024u;
    matrix<double, 2, 2> floorMatrix = floor(matrix<double, 2, 2>(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(floorMatrix[0][0], (lane + 0) % 64, 1) ? 0u : 2048u;
    errors |= matchesExpected(floorMatrix[0][1], (lane + 1) % 64, 1) ? 0u : 2048u;
    errors |= matchesExpected(floorMatrix[1][0], (lane + 2) % 64, 1) ? 0u : 2048u;
    errors |= matchesExpected(floorMatrix[1][1], (lane + 3) % 64, 1) ? 0u : 2048u;
    errors |= matchesExpected(trunc(loadValue(lane)), lane, 2) ? 0u : 4096u;
    errors |= matchesExpected(truncThroughHelper(loadValue(lane)), lane, 2) ? 0u : 8192u;
    double2 truncResult2 = trunc(double2(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64)));
    errors |= matchesExpected(truncResult2.x, (lane + 0) % 64, 2) ? 0u : 16384u;
    errors |= matchesExpected(truncResult2.y, (lane + 1) % 64, 2) ? 0u : 16384u;
    double3 truncResult3 = trunc(double3(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64)));
    errors |= matchesExpected(truncResult3.x, (lane + 0) % 64, 2) ? 0u : 32768u;
    errors |= matchesExpected(truncResult3.y, (lane + 1) % 64, 2) ? 0u : 32768u;
    errors |= matchesExpected(truncResult3.z, (lane + 2) % 64, 2) ? 0u : 32768u;
    double4 truncResult4 = trunc(double4(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(truncResult4.x, (lane + 0) % 64, 2) ? 0u : 65536u;
    errors |= matchesExpected(truncResult4.y, (lane + 1) % 64, 2) ? 0u : 65536u;
    errors |= matchesExpected(truncResult4.z, (lane + 2) % 64, 2) ? 0u : 65536u;
    errors |= matchesExpected(truncResult4.w, (lane + 3) % 64, 2) ? 0u : 65536u;
    matrix<double, 2, 2> truncMatrix = trunc(matrix<double, 2, 2>(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
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
