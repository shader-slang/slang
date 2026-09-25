//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Quiet/signaling NaNs with distinct signs and payloads, followed by both zero signs.
//TEST_INPUT:ubuffer(data=[2143289345 4290772994 2143363909 4290847558 2139095041 4286578690 2139169605 4286653254 0 2147483648], stride=4):name=inputBits
StructuredBuffer<uint> inputBits;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    bool ok = true;
    for (uint partition = 0; partition < 5; ++partition)
    {
        uint mask = 0xFFFFFFFFu;
        uint selectedLane = lane ^ 31u;
        if (partition == 1)
        {
            mask = lane < 16 ? 0xFFFFu : 0xFFFF0000u;
            selectedLane = lane < 16 ? lane ^ 15u : 31;
        }
        if (partition == 2)
        {
            mask = lane < 15 ? 0x7FFFu : 0xFFFF8000u;
            selectedLane = lane < 15 ? 14 : 31;
        }
        if (partition == 3)
        {
            mask = (lane & 1) == 0 ? 0x55555555u : 0xAAAAAAAAu;
            selectedLane = (lane & 1) == 0 ? 30 : 31;
        }
        if (partition == 4)
        {
            mask = 1u << lane;
            selectedLane = lane;
        }
        uint4 members = uint4(mask, 0, 0, 0);
        for (uint family = 0; family < 4; ++family)
        {
            uint first = family == 1 ? 4 : family == 3 ? 8 : 0;
            uint count = family == 2 ? 8 : family == 3 ? 2 : 4;
            uint4 bits, expected;
            for (uint component = 0; component < 4; ++component)
            {
                bits[component] = inputBits[first + (lane + component) % count];
                expected[component] = inputBits[first + (selectedLane + component) % count];
            }
            // Every comparison is unordered or tied, so CUDA selects the second operand.
            // XOR stages select lane^(population-1); an ascending scan selects its last lane.
            // These closed-form indices provide the oracle without another wave operation.
            float4 value = asfloat(bits);
            ok &= asuint(WaveMultiMin(value.x, members)) == expected.x;
            ok &= asuint(WaveMultiMax(value.x, members)) == expected.x;
            float4 minimum = WaveMultiMin(value, members);
            float4 maximum = WaveMultiMax(value, members);
            float2x2 matrixValue = float2x2(value.x, value.y, value.z, value.w);
            float2x2 matrixMinimum = WaveMultiMin(matrixValue, members);
            float2x2 matrixMaximum = WaveMultiMax(matrixValue, members);
            for (uint component = 0; component < 4; ++component)
            {
                ok &= asuint(minimum[component]) == expected[component];
                ok &= asuint(maximum[component]) == expected[component];
                ok &= asuint(matrixMinimum[component / 2][component % 2]) == expected[component];
                ok &= asuint(matrixMaximum[component / 2][component % 2]) == expected[component];
            }
        }
    }
    outputBuffer[lane] = ok ? 6100 + lane : 0;
}

// CHECK: 6100
// CHECK: 6101
// CHECK: 6102
// CHECK: 6103
// CHECK: 6104
// CHECK: 6105
// CHECK: 6106
// CHECK: 6107
// CHECK: 6108
// CHECK: 6109
// CHECK: 6110
// CHECK: 6111
// CHECK: 6112
// CHECK: 6113
// CHECK: 6114
// CHECK: 6115
// CHECK: 6116
// CHECK: 6117
// CHECK: 6118
// CHECK: 6119
// CHECK: 6120
// CHECK: 6121
// CHECK: 6122
// CHECK: 6123
// CHECK: 6124
// CHECK: 6125
// CHECK: 6126
// CHECK: 6127
// CHECK: 6128
// CHECK: 6129
// CHECK: 6130
// CHECK: 6131
