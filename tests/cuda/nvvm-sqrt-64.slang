// Frozen rational sqrt midpoint oracle. Negative/NaN inputs compare by NaN class only.
// Half explicitly models Float32 evaluation then round-to-nearest-even narrowing.
// CUDA hsqrt uses sqrt.approx.ftz.f32: its independent bound is relative error <= 2^-23.
// Half checks CUDA against rationally derived narrowed endpoints and NVVM against exact bits.
// Raw Half scalar bits remain observable so allowed CUDA choices cannot hide baseline transitions.
// 64 live input lanes, 15 comparisons/lane across scalar/helper/vector2/3/4/matrix2x2.
// Six shape-error bits, distinct per-lane completion, and unchanged endpoint guards.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT: ubuffer(data=[0 0 1 0 2 0 3 0 4294967294 1048575 4294967295 1048575 0 1048576 1 1048576 4294967295 1070596095 0 1070596096 1 1070596096 4294967295 1072693247 0 1072693248 1 1072693248 4294967295 1073741823 0 1073741824 1 1073741824 4294967295 1074790399 0 1074790400 1 1074790400 4294967295 1075970047 0 1075970048 1 1075970048 4294967295 1127219199 0 1127219200 1 1127219200 4294967294 2146435071 4294967295 2146435071 0 2146435072 291 2146959360 1 2146435072 0 1073217536 0 2147483648 1 2147483648 2 2147483648 3 2147483648 4294967294 2148532223 4294967295 2148532223 0 2148532224 1 2148532224 4294967295 3218079743 0 3218079744 1 3218079744 4294967295 3220176895 0 3220176896 1 3220176896 4294967295 3221225471 0 3221225472 1 3221225472 4294967295 3222274047 0 3222274048 1 3222274048 4294967295 3223453695 0 3223453696 1 3223453696 4294967295 3274702847 0 3274702848 1 3274702848 4294967294 4293918719 4294967295 4293918719 0 4293918720 291 4294443008 1 4293918720 0 3220701184], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 0 0 0 509607936 0 1719614413 510042270 0 3898100906 510375546 0 4294967294 536870911 0 4294967295 536870911 0 0 536870912 0 0 536870912 0 4294967295 1071644671 0 0 1071644672 0 0 1071644672 0 4294967295 1072693247 0 0 1072693248 0 0 1072693248 0 1719614412 1073127582 0 1719614413 1073127582 0 1719614413 1073127582 0 4294967295 1073741823 0 0 1073741824 0 0 1073741824 0 4294967295 1074266111 0 0 1074266112 0 1 1074266112 0 4294967295 1099956223 0 0 1099956224 0 0 1099956224 0 4294967295 1609564159 0 4294967295 1609564159 0 0 2146435072 0 0 0 1 0 0 1 336142638 1072928910 0 0 2147483648 0 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

double loadValue(uint lane)
{
    return asdouble(inputWords[2 * lane], inputWords[2 * lane + 1]);
}

bool matchesExpected(double value, uint lane)
{
    uint low, high;
    asuint(value, low, high);
    bool nanBits = (high & 0x7ff00000u) == 0x7ff00000u && ((high & 0xfffffu) != 0 || low != 0);
    uint offset = 3 * lane;
    return expectedWords[offset + 2] != 0 ? nanBits :
        low == expectedWords[offset] && high == expectedWords[offset + 1];
}

[noinline]
double sqrtThroughHelper(double value)
{
    return sqrt(value);
}

[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 64) return;
    uint errors = 0;
    double scalarResult = sqrt(loadValue(lane));
    errors |= matchesExpected(scalarResult, lane) ? 0u : 1u;
    errors |= matchesExpected(sqrtThroughHelper(loadValue(lane)), lane) ? 0u : 2u;
    double2 result2 = sqrt(double2(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64)));
    errors |= matchesExpected(result2.x, (lane + 0) % 64) ? 0u : 4u;
    errors |= matchesExpected(result2.y, (lane + 1) % 64) ? 0u : 4u;
    double3 result3 = sqrt(double3(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64)));
    errors |= matchesExpected(result3.x, (lane + 0) % 64) ? 0u : 8u;
    errors |= matchesExpected(result3.y, (lane + 1) % 64) ? 0u : 8u;
    errors |= matchesExpected(result3.z, (lane + 2) % 64) ? 0u : 8u;
    double4 result4 = sqrt(double4(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(result4.x, (lane + 0) % 64) ? 0u : 16u;
    errors |= matchesExpected(result4.y, (lane + 1) % 64) ? 0u : 16u;
    errors |= matchesExpected(result4.z, (lane + 2) % 64) ? 0u : 16u;
    errors |= matchesExpected(result4.w, (lane + 3) % 64) ? 0u : 16u;
    matrix<double, 2, 2> resultMatrix = sqrt(matrix<double, 2, 2>(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(resultMatrix[0][0], (lane + 0) % 64) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[0][1], (lane + 1) % 64) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][0], (lane + 2) % 64) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][1], (lane + 3) % 64) ? 0u : 32u;
    outputBuffer[1 + 2 * lane] = errors;
    outputBuffer[2 + 2 * lane] = 36000 + lane;
}

// CHECK: 324508639{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36000{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36001{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36002{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36003{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36004{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36005{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36006{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36007{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36008{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36009{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36010{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36011{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36012{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36013{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36014{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36015{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36016{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36017{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36018{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36019{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36020{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36021{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36022{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36023{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36024{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36025{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36026{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36027{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36028{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36029{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36030{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36031{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36032{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36033{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36034{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36035{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36036{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36037{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36038{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36039{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36040{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36041{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36042{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36043{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36044{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36045{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36046{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36047{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36048{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36049{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36050{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36051{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36052{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36053{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36054{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36055{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36056{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36057{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36058{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36059{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36060{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36061{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36062{{$}}
// CHECK-NEXT: 0{{$}}
// CHECK-NEXT: 36063{{$}}
// CHECK-NEXT: 3735928559{{$}}
