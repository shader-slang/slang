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

//TEST_INPUT: ubuffer(data=[0 0 1 0 2 0 3 0 8388606 0 8388607 0 8388608 0 8388609 0 1048575999 0 1048576000 0 1048576001 0 1065353215 0 1065353216 0 1065353217 0 1073741823 0 1073741824 0 1073741825 0 1082130431 0 1082130432 0 1082130433 0 1091567615 0 1091567616 0 1091567617 0 1258291199 0 1258291200 0 1258291201 0 2139095038 0 2139095039 0 2139095040 0 2143289635 0 2139095041 0 1069547520 0 2147483648 0 2147483649 0 2147483650 0 2147483651 0 2155872254 0 2155872255 0 2155872256 0 2155872257 0 3196059647 0 3196059648 0 3196059649 0 3212836863 0 3212836864 0 3212836865 0 3221225471 0 3221225472 0 3221225473 0 3229614079 0 3229614080 0 3229614081 0 3239051263 0 3239051264 0 3239051265 0 3405774847 0 3405774848 0 3405774849 0 4286578686 0 4286578687 0 4286578688 0 4290773283 0 4286578689 0 3217031168 0], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 0 0 439682291 0 0 444596224 0 0 446481521 0 0 536870910 0 0 536870911 0 0 536870912 0 0 536870912 0 0 1056964607 0 0 1056964608 0 0 1056964608 0 0 1065353215 0 0 1065353216 0 0 1065353216 0 0 1068827891 0 0 1068827891 0 0 1068827892 0 0 1073741823 0 0 1073741824 0 0 1073741824 0 0 1077936127 0 0 1077936128 0 0 1077936129 0 0 1161102579 0 0 1161102579 0 0 1161102580 0 0 1602224127 0 0 1602224127 0 0 2139095040 0 0 0 0 1 0 0 1 1067238513 0 0 2147483648 0 0 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

float loadValue(uint lane)
{
    return asfloat(inputWords[2 * lane]);
}

bool matchesExpected(float value, uint lane)
{
    uint low = asuint(value);
    uint high = 0;
    bool nanBits = (low & 0x7f800000u) == 0x7f800000u && (low & 0x7fffffu) != 0;
    uint offset = 3 * lane;
    return expectedWords[offset + 2] != 0 ? nanBits :
        low == expectedWords[offset] && high == expectedWords[offset + 1];
}

[noinline]
float sqrtThroughHelper(float value)
{
    return sqrt(value);
}

[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 64) return;
    uint errors = 0;
    float scalarResult = sqrt(loadValue(lane));
    errors |= matchesExpected(scalarResult, lane) ? 0u : 1u;
    errors |= matchesExpected(sqrtThroughHelper(loadValue(lane)), lane) ? 0u : 2u;
    float2 result2 = sqrt(float2(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64)));
    errors |= matchesExpected(result2.x, (lane + 0) % 64) ? 0u : 4u;
    errors |= matchesExpected(result2.y, (lane + 1) % 64) ? 0u : 4u;
    float3 result3 = sqrt(float3(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64)));
    errors |= matchesExpected(result3.x, (lane + 0) % 64) ? 0u : 8u;
    errors |= matchesExpected(result3.y, (lane + 1) % 64) ? 0u : 8u;
    errors |= matchesExpected(result3.z, (lane + 2) % 64) ? 0u : 8u;
    float4 result4 = sqrt(float4(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(result4.x, (lane + 0) % 64) ? 0u : 16u;
    errors |= matchesExpected(result4.y, (lane + 1) % 64) ? 0u : 16u;
    errors |= matchesExpected(result4.z, (lane + 2) % 64) ? 0u : 16u;
    errors |= matchesExpected(result4.w, (lane + 3) % 64) ? 0u : 16u;
    matrix<float, 2, 2> resultMatrix = sqrt(matrix<float, 2, 2>(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
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
