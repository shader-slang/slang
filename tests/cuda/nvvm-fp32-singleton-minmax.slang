//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Runtime-loaded words keep both the operands and independent bitwise oracle dynamic. Every
// lane tests every pattern, including both NaN signs and signaling payloads at lanes 0 and 31.
//TEST_INPUT:ubuffer(data=[0 2147483648 1065353216 3212836864 2139095040 4286578688 2143363909 4291197082 2139169605 4286653254 2147483647 4290772991], stride=4):name=inputBits
StructuredBuffer<uint> inputBits;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    uint4 singleton = uint4(1u << lane, 0, 0, 0);
    bool ok = true;
    for (uint pattern = 0; pattern < 12; ++pattern)
    {
        uint4 bits = uint4(inputBits[pattern], inputBits[(pattern + 1) % 12],
            inputBits[(pattern + 2) % 12], inputBits[(pattern + 3) % 12]);
        float4 value = asfloat(bits);
        ok &= asuint(WaveMultiMin(value.x, singleton)) == bits.x;
        ok &= asuint(WaveMultiMax(value.x, singleton)) == bits.x;
        float4 minimum = WaveMultiMin(value, singleton);
        float4 maximum = WaveMultiMax(value, singleton);
        float2x2 matrixValue = float2x2(value.x, value.y, value.z, value.w);
        float2x2 matrixMinimum = WaveMultiMin(matrixValue, singleton);
        float2x2 matrixMaximum = WaveMultiMax(matrixValue, singleton);
        for (uint component = 0; component < 4; ++component)
        {
            ok &= asuint(minimum[component]) == bits[component];
            ok &= asuint(maximum[component]) == bits[component];
            ok &= asuint(matrixMinimum[component / 2][component % 2]) == bits[component];
            ok &= asuint(matrixMaximum[component / 2][component % 2]) == bits[component];
        }
    }

    // Finite nonsingleton neighbors retain the existing reduction. All named lanes execute
    // with the same mask, including high/low partitions and disjoint sparse partitions.
    for (uint partition = 0; partition < 4; ++partition)
    {
        uint mask = 0xFFFFFFFFu;
        uint first = 0;
        uint last = 31;
        if (partition == 1)
        {
            mask = lane < 16 ? 0xFFFFu : 0xFFFF0000u;
            first = lane < 16 ? 0 : 16;
            last = first + 15;
        }
        if (partition == 2)
        {
            mask = lane < 15 ? 0x7FFFu : 0xFFFF8000u;
            first = lane < 15 ? 0 : 15;
            last = lane < 15 ? 14 : 31;
        }
        if (partition == 3)
        {
            mask = (lane & 1) == 0 ? 0x55555555u : 0xAAAAAAAAu;
            first = lane & 1;
            last = first + 30;
        }
        uint4 members = uint4(mask, 0, 0, 0);
        float value = float(lane) - 16.0;
        float low = float(first) - 16.0;
        float high = float(last) - 16.0;
        ok &= WaveMultiMin(value, members) == low;
        ok &= WaveMultiMax(value, members) == high;
        float2 values = float2(value, -value);
        float2 minimum = WaveMultiMin(values, members);
        float2 maximum = WaveMultiMax(values, members);
        ok &= all(minimum == float2(low, -high));
        ok &= all(maximum == float2(high, -low));
        float2x2 matrixValue = float2x2(value, -value, value + 64.0, -value - 64.0);
        float2x2 matrixMinimum = WaveMultiMin(matrixValue, members);
        float2x2 matrixMaximum = WaveMultiMax(matrixValue, members);
        ok &= all(matrixMinimum[0] == float2(low, -high));
        ok &= all(matrixMinimum[1] == float2(low + 64.0, -high - 64.0));
        ok &= all(matrixMaximum[0] == float2(high, -low));
        ok &= all(matrixMaximum[1] == float2(high + 64.0, -low - 64.0));
    }
    outputBuffer[lane] = ok ? 6000 + lane : 0;
}

// CHECK: 6000
// CHECK: 6001
// CHECK: 6002
// CHECK: 6003
// CHECK: 6004
// CHECK: 6005
// CHECK: 6006
// CHECK: 6007
// CHECK: 6008
// CHECK: 6009
// CHECK: 6010
// CHECK: 6011
// CHECK: 6012
// CHECK: 6013
// CHECK: 6014
// CHECK: 6015
// CHECK: 6016
// CHECK: 6017
// CHECK: 6018
// CHECK: 6019
// CHECK: 6020
// CHECK: 6021
// CHECK: 6022
// CHECK: 6023
// CHECK: 6024
// CHECK: 6025
// CHECK: 6026
// CHECK: 6027
// CHECK: 6028
// CHECK: 6029
// CHECK: 6030
// CHECK: 6031
