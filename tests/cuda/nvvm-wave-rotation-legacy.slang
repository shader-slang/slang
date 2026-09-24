//TEST:SIMPLE(filecheck=ROTATE):-target cuda -capability cuda_sm_5_0 -entry rotateMain -stage compute
//TEST:SIMPLE(filecheck=ROTATE):-target cuda -capability cuda_sm_6_0 -entry rotateMain -stage compute
//TEST:SIMPLE(filecheck=CLUSTERED):-target cuda -capability cuda_sm_5_0 -entry clusteredMain -stage compute
//TEST:SIMPLE(filecheck=CLUSTERED):-target cuda -capability cuda_sm_6_0 -entry clusteredMain -stage compute

// Lower CUDA targets retain the original scalar and vector intrinsic implementations.
// ROTATE-DAG: __shfl_sync(0xFFFFFFFF
// ROTATE-DAG: _slang_waveRotate(
// CLUSTERED: _slang_waveClusteredRotate(
RWStructuredBuffer<uint4> outputBuffer;

[numthreads(32, 1, 1)]
void rotateMain(uint3 tid : SV_DispatchThreadID)
{
    uint4 value = uint4(tid.x, tid.x + 32, tid.x + 64, tid.x + 96);
    uint64_t wide = (uint64_t(tid.x + 128) << 32) | uint64_t(tid.x);
    uint64_t rotated = WaveRotate(wide, 3);
    outputBuffer[tid.x] = WaveRotate(value, 3) + uint4(uint(rotated), uint(rotated >> 32), 0, 0);
}

[numthreads(32, 1, 1)]
void clusteredMain(uint3 tid : SV_DispatchThreadID)
{
    uint4 value = uint4(tid.x, tid.x + 32, tid.x + 64, tid.x + 96);
    uint64_t wide = (uint64_t(tid.x + 128) << 32) | uint64_t(tid.x);
    uint64_t rotated = WaveClusteredRotate(wide, 3, 8);
    outputBuffer[tid.x] = WaveClusteredRotate(value, 3, 8)
        + uint4(uint(rotated), uint(rotated >> 32), 0, 0);
}
