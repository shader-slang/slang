// Frozen rational sqrt midpoint oracle. Negative/NaN inputs compare by NaN class only.
// Half explicitly models Float32 evaluation then round-to-nearest-even narrowing.
// CUDA hsqrt uses sqrt.approx.ftz.f32: its independent bound is relative error <= 2^-23.
// Half checks CUDA against rationally derived narrowed endpoints and NVVM against exact bits.
// Raw Half scalar bits remain observable so allowed CUDA choices cannot hide baseline transitions.
// 64 live input lanes, 15 comparisons/lane across scalar/helper/vector2/3/4/matrix2x2.
// Six shape-error bits, distinct per-lane completion, and unchanged endpoint guards.

//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=CUDA): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=NVVM): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=NVVM): -cuda -compute -shaderobj -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST_INPUT: ubuffer(data=[0 0 1 0 2 0 3 0 1022 0 1023 0 1024 0 1025 0 13311 0 13312 0 13313 0 15359 0 15360 0 15361 0 16383 0 16384 0 16385 0 17407 0 17408 0 17409 0 18559 0 18560 0 18561 0 25599 0 25600 0 25601 0 31742 0 31743 0 31744 0 32547 0 31745 0 15872 0 32768 0 32769 0 32770 0 32771 0 33790 0 33791 0 33792 0 33793 0 46079 0 46080 0 46081 0 48127 0 48128 0 48129 0 49151 0 49152 0 49153 0 50175 0 50176 0 50177 0 51327 0 51328 0 51329 0 58367 0 58368 0 58369 0 64510 0 64511 0 64512 0 65315 0 64513 0 48640 0], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 0 0 3072 0 0 3496 0 0 3822 0 0 8190 0 0 8191 0 0 8192 0 0 8192 0 0 14335 0 0 14336 0 0 14336 0 0 15359 0 0 15360 0 0 15360 0 0 15784 0 0 15784 0 0 15785 0 0 16383 0 0 16384 0 0 16384 0 0 16895 0 0 16896 0 0 16897 0 0 20479 0 0 20480 0 0 20480 0 0 23551 0 0 23551 0 0 31744 0 0 0 0 1 0 0 1 15590 0 0 32768 0 0 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1 0 0 1], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[0 0 3072 3072 3496 3496 3822 3822 8190 8190 8191 8191 8192 8192 8192 8193 14335 14336 14336 14336 14336 14337 15359 15360 15360 15360 15360 15361 15784 15784 15784 15784 15785 15785 16383 16384 16384 16384 16384 16385 16895 16895 16896 16896 16897 16897 20479 20480 20480 20480 20480 20481 23551 23551 23551 23552 31744 31744 0 0 0 0 15590 15590 32768 32768 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):name=cudaBounds
StructuredBuffer<uint> cudaBounds;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

half loadValue(uint lane)
{
    return bit_cast<half>(uint16_t(inputWords[2 * lane]));
}

bool usesPreciseNVVMOracle()
{
    __target_switch
    {
    case nvvm: return true;
    default: return false;
    }
}

bool matchesExpected(half value, uint lane)
{
    uint low = uint(bit_cast<uint16_t>(value));
    uint high = 0;
    bool nanBits = (low & 0x7c00u) == 0x7c00u && (low & 0x3ffu) != 0;
    uint offset = 3 * lane;
    if (expectedWords[offset + 2] != 0) return nanBits;
    return usesPreciseNVVMOracle() ? low == expectedWords[offset] :
        low >= cudaBounds[2 * lane] && low <= cudaBounds[2 * lane + 1];
}

[noinline]
half sqrtThroughHelper(half value)
{
    return sqrt(value);
}

[numthreads(64, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 64) return;
    uint errors = 0;
    half scalarResult = sqrt(loadValue(lane));
    errors |= matchesExpected(scalarResult, lane) ? 0u : 1u;
    errors |= matchesExpected(sqrtThroughHelper(loadValue(lane)), lane) ? 0u : 2u;
    half2 result2 = sqrt(half2(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64)));
    errors |= matchesExpected(result2.x, (lane + 0) % 64) ? 0u : 4u;
    errors |= matchesExpected(result2.y, (lane + 1) % 64) ? 0u : 4u;
    half3 result3 = sqrt(half3(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64)));
    errors |= matchesExpected(result3.x, (lane + 0) % 64) ? 0u : 8u;
    errors |= matchesExpected(result3.y, (lane + 1) % 64) ? 0u : 8u;
    errors |= matchesExpected(result3.z, (lane + 2) % 64) ? 0u : 8u;
    half4 result4 = sqrt(half4(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(result4.x, (lane + 0) % 64) ? 0u : 16u;
    errors |= matchesExpected(result4.y, (lane + 1) % 64) ? 0u : 16u;
    errors |= matchesExpected(result4.z, (lane + 2) % 64) ? 0u : 16u;
    errors |= matchesExpected(result4.w, (lane + 3) % 64) ? 0u : 16u;
    matrix<half, 2, 2> resultMatrix = sqrt(matrix<half, 2, 2>(loadValue((lane + 0) % 64), loadValue((lane + 1) % 64), loadValue((lane + 2) % 64), loadValue((lane + 3) % 64)));
    errors |= matchesExpected(resultMatrix[0][0], (lane + 0) % 64) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[0][1], (lane + 1) % 64) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][0], (lane + 2) % 64) ? 0u : 32u;
    errors |= matchesExpected(resultMatrix[1][1], (lane + 3) % 64) ? 0u : 32u;
    if (lane == 0) outputBuffer[1] = usesPreciseNVVMOracle() ? 36037u : 1209u;
    outputBuffer[2 + 3 * lane] = errors;
    outputBuffer[3 + 3 * lane] = uint(bit_cast<uint16_t>(scalarResult));
    outputBuffer[4 + 3 * lane] = 36000 + lane;
}

// CUDA: 324508639{{$}}
// CUDA-NEXT: 1209{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 36000{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 3072{{$}}
// CUDA-NEXT: 36001{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 3496{{$}}
// CUDA-NEXT: 36002{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 3822{{$}}
// CUDA-NEXT: 36003{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 8190{{$}}
// CUDA-NEXT: 36004{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 8191{{$}}
// CUDA-NEXT: 36005{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 8192{{$}}
// CUDA-NEXT: 36006{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{(8192|8193)}}{{$}}
// CUDA-NEXT: 36007{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{(14335|14336)}}{{$}}
// CUDA-NEXT: 36008{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 14336{{$}}
// CUDA-NEXT: 36009{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{(14336|14337)}}{{$}}
// CUDA-NEXT: 36010{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{(15359|15360)}}{{$}}
// CUDA-NEXT: 36011{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 15360{{$}}
// CUDA-NEXT: 36012{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{(15360|15361)}}{{$}}
// CUDA-NEXT: 36013{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 15784{{$}}
// CUDA-NEXT: 36014{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 15784{{$}}
// CUDA-NEXT: 36015{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 15785{{$}}
// CUDA-NEXT: 36016{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{(16383|16384)}}{{$}}
// CUDA-NEXT: 36017{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 16384{{$}}
// CUDA-NEXT: 36018{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{(16384|16385)}}{{$}}
// CUDA-NEXT: 36019{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 16895{{$}}
// CUDA-NEXT: 36020{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 16896{{$}}
// CUDA-NEXT: 36021{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 16897{{$}}
// CUDA-NEXT: 36022{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{(20479|20480)}}{{$}}
// CUDA-NEXT: 36023{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 20480{{$}}
// CUDA-NEXT: 36024{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{(20480|20481)}}{{$}}
// CUDA-NEXT: 36025{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 23551{{$}}
// CUDA-NEXT: 36026{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{(23551|23552)}}{{$}}
// CUDA-NEXT: 36027{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 31744{{$}}
// CUDA-NEXT: 36028{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36029{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36030{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 15590{{$}}
// CUDA-NEXT: 36031{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: 32768{{$}}
// CUDA-NEXT: 36032{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36033{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36034{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36035{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36036{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36037{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36038{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36039{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36040{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36041{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36042{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36043{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36044{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36045{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36046{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36047{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36048{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36049{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36050{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36051{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36052{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36053{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36054{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36055{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36056{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36057{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36058{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36059{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36060{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36061{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36062{{$}}
// CUDA-NEXT: 0{{$}}
// CUDA-NEXT: {{[0-9]+}}{{$}}
// CUDA-NEXT: 36063{{$}}
// CUDA-NEXT: 3735928559{{$}}
// NVVM: 324508639{{$}}
// NVVM-NEXT: 36037{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 36000{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 3072{{$}}
// NVVM-NEXT: 36001{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 3496{{$}}
// NVVM-NEXT: 36002{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 3822{{$}}
// NVVM-NEXT: 36003{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 8190{{$}}
// NVVM-NEXT: 36004{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 8191{{$}}
// NVVM-NEXT: 36005{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 8192{{$}}
// NVVM-NEXT: 36006{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 8192{{$}}
// NVVM-NEXT: 36007{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 14335{{$}}
// NVVM-NEXT: 36008{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 14336{{$}}
// NVVM-NEXT: 36009{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 14336{{$}}
// NVVM-NEXT: 36010{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 15359{{$}}
// NVVM-NEXT: 36011{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 15360{{$}}
// NVVM-NEXT: 36012{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 15360{{$}}
// NVVM-NEXT: 36013{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 15784{{$}}
// NVVM-NEXT: 36014{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 15784{{$}}
// NVVM-NEXT: 36015{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 15785{{$}}
// NVVM-NEXT: 36016{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 16383{{$}}
// NVVM-NEXT: 36017{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 16384{{$}}
// NVVM-NEXT: 36018{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 16384{{$}}
// NVVM-NEXT: 36019{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 16895{{$}}
// NVVM-NEXT: 36020{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 16896{{$}}
// NVVM-NEXT: 36021{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 16897{{$}}
// NVVM-NEXT: 36022{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 20479{{$}}
// NVVM-NEXT: 36023{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 20480{{$}}
// NVVM-NEXT: 36024{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 20480{{$}}
// NVVM-NEXT: 36025{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 23551{{$}}
// NVVM-NEXT: 36026{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 23551{{$}}
// NVVM-NEXT: 36027{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 31744{{$}}
// NVVM-NEXT: 36028{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36029{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36030{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 15590{{$}}
// NVVM-NEXT: 36031{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: 32768{{$}}
// NVVM-NEXT: 36032{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36033{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36034{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36035{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36036{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36037{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36038{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36039{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36040{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36041{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36042{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36043{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36044{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36045{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36046{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36047{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36048{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36049{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36050{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36051{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36052{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36053{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36054{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36055{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36056{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36057{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36058{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36059{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36060{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36061{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36062{{$}}
// NVVM-NEXT: 0{{$}}
// NVVM-NEXT: {{[0-9]+}}{{$}}
// NVVM-NEXT: 36063{{$}}
// NVVM-NEXT: 3735928559{{$}}
