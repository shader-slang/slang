// Pin the capability -> `-arch` -> emitted `.target` pathway.
//
// This pins the *pre-existing* mapping only, and deliberately so: `cuda_sm_8_0`
// is a real architecture in every supported toolkit, so it is neither raised by
// the minimum-architecture logic nor clamped by the maximum nor rounded to a
// neighbour. It therefore produces `sm_80` on every path, and would still pass
// if the resolution logic were removed.
//
// It earns its place because nothing else in the test suite asserts that a
// requested capability reaches the emitted PTX at all, which is how a silent
// mis-mapping in this area went unnoticed. The resolution logic itself is
// covered by tools/slang-unit-test/unit-test-nvrtc-arch-resolve.cpp, which can
// exercise the cases that no `-capability` value can reach on current toolkits.

//TEST:SIMPLE(filecheck=PTX): -target ptx -capability cuda_sm_8_0 -entry computeMain -stage compute

// PTX: .target sm_80

RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[tid.x] = 1;
}
