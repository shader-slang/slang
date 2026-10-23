//TEST_CATEGORY(wave, compute)
//TEST:COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-wgpu -compute -shaderobj

// Runs the WGSL emulation of WaveMatch (one subgroupShuffle and one subgroupBallot per lane) on
// 8 threads, so the lanes above 7 of the subgroup are inactive. Each row is the WaveMatch result
// (its first component) of threads 0..7:
// - a uint (i & 1) and a uint2 (i & 1, i >> 2) partition the threads by value;
// - a bool (i < 3) is compared as 0/1;
// - a NaN never equals itself, but WaveMatch always includes the current lane, so thread 5 (NaN)
//   matches only itself and the other threads (1.0) match each other.
// The other components are 0: the subgroup has at least 8 lanes.

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4):out,name outputBuffer
RWStructuredBuffer<uint> outputBuffer;

// NaN bits read from a buffer, so the value is not a constant.
//TEST_INPUT:ubuffer(data=[2143289344], stride=4):name nanBuffer
RWStructuredBuffer<uint> nanBuffer;

// CHECK: 55
// CHECK-NEXT: AA
// CHECK-NEXT: 55
// CHECK-NEXT: AA
// CHECK-NEXT: 55
// CHECK-NEXT: AA
// CHECK-NEXT: 55
// CHECK-NEXT: AA
// CHECK-NEXT: 5
// CHECK-NEXT: A
// CHECK-NEXT: 5
// CHECK-NEXT: A
// CHECK-NEXT: 50
// CHECK-NEXT: A0
// CHECK-NEXT: 50
// CHECK-NEXT: A0
// CHECK-NEXT: 7
// CHECK-NEXT: 7
// CHECK-NEXT: 7
// CHECK-NEXT: F8
// CHECK-NEXT: F8
// CHECK-NEXT: F8
// CHECK-NEXT: F8
// CHECK-NEXT: F8
// CHECK-NEXT: DF
// CHECK-NEXT: DF
// CHECK-NEXT: DF
// CHECK-NEXT: DF
// CHECK-NEXT: DF
// CHECK-NEXT: 20
// CHECK-NEXT: DF
// CHECK-NEXT: DF
// CHECK-NEXT: 0
[numthreads(8, 1, 1)]
[shader("compute")]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const uint i = dispatchThreadID.x;
    const float value = i == 5 ? asfloat(nanBuffer[0]) : 1.0f;

    const uint4 byUInt = WaveMatch(i & 1);
    const uint4 byVector = WaveMatch(uint2(i & 1, i >> 2));
    const uint4 byBool = WaveMatch(i < 3);
    const uint4 byFloat = WaveMatch(value);

    outputBuffer[i] = byUInt.x;
    outputBuffer[8 + i] = byVector.x;
    outputBuffer[16 + i] = byBool.x;
    outputBuffer[24 + i] = byFloat.x;
    // Thread 0 records whether any result has a bit set above lane 31.
    if (i == 0)
        outputBuffer[32] = any(uint3(byUInt.y, byVector.z, byFloat.w) != 0) ? 1 : 0;
}
