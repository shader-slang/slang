// Pin the capability -> `-arch` -> emitted `.target` pathway.
//
// `cuda_sm_8_0` is chosen because it is stable across NVRTC versions: it is well
// above any plausible reported floor and well below the ceiling, so it is neither
// raised by the minimum-architecture logic nor clamped by the maximum, and it is a
// real architecture in every supported toolkit rather than one that has to be
// rounded to a neighbour.
//
// Nothing else in the test suite asserts that a requested capability reaches the
// emitted PTX, which is how a silent mis-mapping in this area went unnoticed.

//TEST:SIMPLE(filecheck=PTX): -target ptx -capability cuda_sm_8_0 -entry computeMain -stage compute

// PTX: .target sm_80

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = 1;
}
