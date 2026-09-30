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

//TEST_INPUT: ubuffer(data=[0 0 1 0 2 0 8388607 0 8388608 0 8388609 0 976789168 0 976789169 0 976789170 0 976789171 0 990537145 0 990537146 0 990537147 0 990537148 0 1048576000 0 1056964608 0 1065353214 0 1065353215 0 1065353216 0 1065353217 0 1073741822 0 1073741823 0 1073741824 0 1073741825 0 1082130432 0 1090519040 0 1098907648 0 1123942400 0 1124073470 0 1124073471 0 1124073472 0 1124073473 0 2139095039 0 2139095040 0 2139095041 0 2143289344 0 2147483648 0 2147483649 0 2147483650 0 2155872255 0 2155872256 0 2155872257 0 3196059648 0 3204448256 0 3212836863 0 3212836864 0 3212836865 0 3212836866 0 3221225472 0 3229614080 0 3238002688 0 3246391296 0 3254779904 0 3263168512 0 3267887104 0 3271294975 0 3271294976 0 3271294977 0 3271294978 0 3272933376 0 3272998911 0 3272998912 0 3272998913 0 3272998914 0 3291414528 0 4286578687 0 4286578688 0 4286578689 0 4290772992 0], stride=4):name=inputWords
StructuredBuffer<uint> inputWords;

//TEST_INPUT: ubuffer(data=[0 1 1065353216 0 0 0 0 0 0 0 0 0 0 0 0 0 0 7 1065353212 0 1065353213 0 1065353214 0 1065353215 0 1065353216 0 1065353217 0 1065353218 0 0 7 1065353212 0 1065353213 0 1065353214 0 1065353215 0 1065353216 0 1065353217 0 1065353218 0 0 7 1065353212 0 1065353213 0 1065353214 0 1065353215 0 1065353216 0 1065353217 0 1065353218 0 0 7 1065353212 0 1065353213 0 1065353214 0 1065353215 0 1065353216 0 1065353217 0 1065353218 0 0 7 1065353212 0 1065353213 0 1065353214 0 1065353215 0 1065353216 0 1065353217 0 1065353218 0 0 5 1065357310 0 1065357311 0 1065357312 0 1065357313 0 1065357314 0 0 0 0 0 0 5 1065357310 0 1065357311 0 1065357312 0 1065357313 0 1065357314 0 0 0 0 0 0 5 1065357310 0 1065357311 0 1065357312 0 1065357313 0 1065357314 0 0 0 0 0 0 5 1065357310 0 1065357311 0 1065357312 0 1065357313 0 1065357314 0 0 0 0 0 0 5 1065365502 0 1065365503 0 1065365504 0 1065365505 0 1065365506 0 0 0 0 0 0 5 1065365502 0 1065365503 0 1065365504 0 1065365505 0 1065365506 0 0 0 0 0 0 5 1065365502 0 1065365503 0 1065365504 0 1065365505 0 1065365506 0 0 0 0 0 0 5 1065365502 0 1065365503 0 1065365504 0 1065365505 0 1065365506 0 0 0 0 0 0 5 1066940398 0 1066940399 0 1066940400 0 1066940401 0 1066940402 0 0 0 0 0 0 5 1068827889 0 1068827890 0 1068827891 0 1068827892 0 1068827893 0 0 0 0 0 0 5 1073741821 0 1073741822 0 1073741823 0 1073741824 0 1073741825 0 0 0 0 0 0 5 1073741821 0 1073741822 0 1073741823 0 1073741824 0 1073741825 0 0 0 0 0 0 7 1073741820 0 1073741821 0 1073741822 0 1073741823 0 1073741824 0 1073741825 0 1073741826 0 0 6 1073741822 0 1073741823 0 1073741824 0 1073741825 0 1073741826 0 1073741827 0 0 0 0 5 1082130427 0 1082130428 0 1082130429 0 1082130430 0 1082130431 0 0 0 0 0 0 5 1082130429 0 1082130430 0 1082130431 0 1082130432 0 1082130433 0 0 0 0 0 0 7 1082130428 0 1082130429 0 1082130430 0 1082130431 0 1082130432 0 1082130433 0 1082130434 0 0 6 1082130430 0 1082130431 0 1082130432 0 1082130433 0 1082130434 0 1082130435 0 0 0 0 7 1098907644 0 1098907645 0 1098907646 0 1098907647 0 1098907648 0 1098907649 0 1098907650 0 0 7 1132462076 0 1132462077 0 1132462078 0 1132462079 0 1132462080 0 1132462081 0 1132462082 0 0 7 1199570940 0 1199570941 0 1199570942 0 1199570943 0 1199570944 0 1199570945 0 1199570946 0 0 7 2130706428 0 2130706429 0 2130706430 0 2130706431 0 2130706432 0 2130706433 0 2130706434 0 0 5 2139094861 0 2139094862 0 2139094863 0 2139094864 0 2139094865 0 0 0 0 0 0 5 2139094949 0 2139094950 0 2139094951 0 2139094952 0 2139094953 0 0 0 0 0 0 3 2139095038 0 2139095039 0 2139095040 0 0 0 0 0 0 0 0 0 0 3 2139095038 0 2139095039 0 2139095040 0 0 0 0 0 0 0 0 0 0 3 2139095038 0 2139095039 0 2139095040 0 0 0 0 0 0 0 0 0 0 1 2139095040 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1065353216 0 0 0 0 0 0 0 0 0 0 0 0 0 0 7 1065353212 0 1065353213 0 1065353214 0 1065353215 0 1065353216 0 1065353217 0 1065353218 0 0 7 1065353212 0 1065353213 0 1065353214 0 1065353215 0 1065353216 0 1065353217 0 1065353218 0 0 7 1065353212 0 1065353213 0 1065353214 0 1065353215 0 1065353216 0 1065353217 0 1065353218 0 0 7 1065353212 0 1065353213 0 1065353214 0 1065353215 0 1065353216 0 1065353217 0 1065353218 0 0 7 1065353212 0 1065353213 0 1065353214 0 1065353215 0 1065353216 0 1065353217 0 1065353218 0 0 5 1062683899 0 1062683900 0 1062683901 0 1062683902 0 1062683903 0 0 0 0 0 0 5 1060439281 0 1060439282 0 1060439283 0 1060439284 0 1060439285 0 0 0 0 0 0 7 1056964604 0 1056964605 0 1056964606 0 1056964607 0 1056964608 0 1056964609 0 1056964610 0 0 7 1056964604 0 1056964605 0 1056964606 0 1056964607 0 1056964608 0 1056964609 0 1056964610 0 0 5 1056964605 0 1056964606 0 1056964607 0 1056964608 0 1056964609 0 0 0 0 0 0 5 1056964603 0 1056964604 0 1056964605 0 1056964606 0 1056964607 0 0 0 0 0 0 7 1048575996 0 1048575997 0 1048575998 0 1048575999 0 1048576000 0 1048576001 0 1048576002 0 0 7 1031798780 0 1031798781 0 1031798782 0 1031798783 0 1031798784 0 1031798785 0 1031798786 0 0 7 998244348 0 998244349 0 998244350 0 998244351 0 998244352 0 998244353 0 998244354 0 0 7 931135484 0 931135485 0 931135486 0 931135487 0 931135488 0 931135489 0 931135490 0 0 7 796917756 0 796917757 0 796917758 0 796917759 0 796917760 0 796917761 0 796917762 0 0 7 528482300 0 528482301 0 528482302 0 528482303 0 528482304 0 528482305 0 528482306 0 0 7 226492412 0 226492413 0 226492414 0 226492415 0 226492416 0 226492417 0 226492418 0 0 5 8388650 0 8388651 0 8388652 0 8388653 0 8388654 0 0 0 0 0 0 5 8388606 0 8388607 0 8388608 0 8388609 0 8388610 0 0 0 0 0 0 5 8388562 0 8388563 0 8388564 0 8388565 0 8388566 0 0 0 0 0 0 5 8388517 0 8388518 0 8388519 0 8388520 0 8388521 0 0 0 0 0 0 4 0 0 1 0 2 0 3 0 0 0 0 0 0 0 0 4 0 0 1 0 2 0 3 0 0 0 0 0 0 0 0 3 0 0 1 0 2 0 0 0 0 0 0 0 0 0 0 3 0 0 1 0 2 0 0 0 0 0 0 0 0 0 0 3 0 0 1 0 2 0 0 0 0 0 0 0 0 0 0 3 0 0 1 0 2 0 0 0 0 0 0 0 0 0 0 3 0 0 1 0 2 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):name=expectedWords
StructuredBuffer<uint> expectedWords;

//TEST_INPUT: ubuffer(data=[324508639 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 3735928559], stride=4):out,name=outputBuffer
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
float exp2ThroughHelper(float value)
{
    return exp2(value);
}

[numthreads(128, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    if (lane >= 69) return;
    uint errors = 0;
    float scalarResult = exp2(loadValue(lane));
    errors |= matchesExpected(scalarResult, lane) ? 0u : 1u;
    errors |= matchesExpected(exp2ThroughHelper(loadValue(lane)), lane) ? 0u : 2u;
    float2 result2 = exp2(float2(loadValue((lane + 0) % 69), loadValue((lane + 1) % 69)));
    errors |= matchesExpected(result2.x, (lane + 0) % 69) ? 0u : 4u;
    errors |= matchesExpected(result2.y, (lane + 1) % 69) ? 0u : 4u;
    float3 result3 = exp2(float3(loadValue((lane + 0) % 69), loadValue((lane + 1) % 69), loadValue((lane + 2) % 69)));
    errors |= matchesExpected(result3.x, (lane + 0) % 69) ? 0u : 8u;
    errors |= matchesExpected(result3.y, (lane + 1) % 69) ? 0u : 8u;
    errors |= matchesExpected(result3.z, (lane + 2) % 69) ? 0u : 8u;
    float4 result4 = exp2(float4(loadValue((lane + 0) % 69), loadValue((lane + 1) % 69), loadValue((lane + 2) % 69), loadValue((lane + 3) % 69)));
    errors |= matchesExpected(result4.x, (lane + 0) % 69) ? 0u : 16u;
    errors |= matchesExpected(result4.y, (lane + 1) % 69) ? 0u : 16u;
    errors |= matchesExpected(result4.z, (lane + 2) % 69) ? 0u : 16u;
    errors |= matchesExpected(result4.w, (lane + 3) % 69) ? 0u : 16u;
    matrix<float, 2, 2> resultMatrix = exp2(matrix<float, 2, 2>(loadValue((lane + 0) % 69), loadValue((lane + 1) % 69), loadValue((lane + 2) % 69), loadValue((lane + 3) % 69)));
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
