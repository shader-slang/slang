//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -O3
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -output-using-type -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Each lane owns one output. Expected source lanes are supplied independently by this table:
// columns are a full-wave rotation by 33 and an eight-lane clustered rotation by 11.
//TEST_INPUT:ubuffer(data=[1 3 2 4 3 5 4 6 5 7 6 0 7 1 8 2 9 11 10 12 11 13 12 14 13 15 14 8 15 9 16 10 17 19 18 20 19 21 20 22 21 23 22 16 23 17 24 18 25 27 26 28 27 29 28 30 29 31 30 24 31 25 0 26], stride=8):name=expectedLanes
StructuredBuffer<uint2> expectedLanes;
//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name=outputBuffer
RWStructuredBuffer<uint> outputBuffer;

__generic<T : __BuiltinArithmeticType, let N : int>
bool checkVectors(uint lane, uint rotated, uint clustered, T base)
{
    vector<T, N> value;
    vector<T, N> expectedRotate;
    vector<T, N> expectedCluster;
    [ForceUnroll]
    for (int i = 0; i < N; ++i)
    {
        value[i] = base + T((lane + 1) * (i + 1));
        expectedRotate[i] = base + T((rotated + 1) * (i + 1));
        expectedCluster[i] = base + T((clustered + 1) * (i + 1));
    }
    return all(WaveRotate(value, 33) == expectedRotate)
        & all(WaveClusteredRotate(value, 11, 8) == expectedCluster);
}

__generic<T : __BuiltinArithmeticType>
bool checkFamily(uint lane, uint rotated, uint clustered, T base)
{
    T value = base + T(lane);
    return (WaveRotate(value, 33) == base + T(rotated))
        & (WaveClusteredRotate(value, 11, 8) == base + T(clustered))
        & checkVectors<T, 2>(lane, rotated, clustered, base)
        & checkVectors<T, 3>(lane, rotated, clustered, base)
        & checkVectors<T, 4>(lane, rotated, clustered, base);
}

bool4 laneFlags(uint lane)
{
    return bool4((lane & 1) != 0, (lane & 2) != 0, (lane & 4) != 0, (lane & 8) != 0);
}

[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint lane = tid.x;
    uint rotated = expectedLanes[lane].x;
    uint clustered = expectedLanes[lane].y;
    bool ok = checkFamily<uint8_t>(lane, rotated, clustered, uint8_t(128))
        & checkFamily<int8_t>(lane, rotated, clustered, int8_t(-100))
        & checkFamily<uint16_t>(lane, rotated, clustered, uint16_t(32768))
        & checkFamily<int16_t>(lane, rotated, clustered, int16_t(-20000))
        & checkFamily<uint>(lane, rotated, clustered, 0x80000000u)
        & checkFamily<int>(lane, rotated, clustered, -100000)
        & checkFamily<uint64_t>(lane, rotated, clustered, uint64_t(0xFEDCBA98u) << 32)
        & checkFamily<int64_t>(lane, rotated, clustered, -(int64_t(0x12345678u) << 32))
        & checkFamily<half>(lane, rotated, clustered, half(-64))
        & checkFamily<float>(lane, rotated, clustered, -64.0f);
    bool4 flags = laneFlags(lane);
    bool4 rotatedFlags = laneFlags(rotated);
    bool4 clusteredFlags = laneFlags(clustered);
    ok &= WaveRotate(flags.x, 33) == rotatedFlags.x;
    ok &= WaveClusteredRotate(flags.x, 11, 8) == clusteredFlags.x;
    ok &= all(WaveRotate(flags.xy, 33) == rotatedFlags.xy);
    ok &= all(WaveRotate(flags.xyz, 33) == rotatedFlags.xyz);
    ok &= all(WaveRotate(flags, 33) == rotatedFlags);
    ok &= all(WaveClusteredRotate(flags.xy, 11, 8) == clusteredFlags.xy);
    ok &= all(WaveClusteredRotate(flags.xyz, 11, 8) == clusteredFlags.xyz);
    ok &= all(WaveClusteredRotate(flags, 11, 8) == clusteredFlags);
    // Identity and wraparound boundaries use equivalences independent of the library formula.
    ok &= WaveRotate(lane, 0) == lane;
    ok &= WaveRotate(lane, 32) == lane;
    ok &= WaveRotate(lane, 1) == rotated;
    uint predecessor = lane == 0 ? 31 : lane - 1;
    ok &= WaveRotate(lane, 31) == predecessor;
    ok &= WaveRotate(lane, 0xFFFFFFFFu) == predecessor;
    ok &= WaveClusteredRotate(lane, 33, 1) == lane;
    ok &= WaveClusteredRotate(lane, 1, 2) == (lane ^ 1);
    ok &= WaveClusteredRotate(lane, 0, 4) == lane;
    ok &= WaveClusteredRotate(lane, 4, 4) == lane;
    ok &= WaveClusteredRotate(lane, 8, 16) == (lane ^ 8);
    ok &= WaveClusteredRotate(lane, 33, 32) == rotated;
    outputBuffer[lane] = ok ? 1000 + lane : 0;
}

// CHECK: 1000
// CHECK: 1001
// CHECK: 1002
// CHECK: 1003
// CHECK: 1004
// CHECK: 1005
// CHECK: 1006
// CHECK: 1007
// CHECK: 1008
// CHECK: 1009
// CHECK: 1010
// CHECK: 1011
// CHECK: 1012
// CHECK: 1013
// CHECK: 1014
// CHECK: 1015
// CHECK: 1016
// CHECK: 1017
// CHECK: 1018
// CHECK: 1019
// CHECK: 1020
// CHECK: 1021
// CHECK: 1022
// CHECK: 1023
// CHECK: 1024
// CHECK: 1025
// CHECK: 1026
// CHECK: 1027
// CHECK: 1028
// CHECK: 1029
// CHECK: 1030
// CHECK: 1031
