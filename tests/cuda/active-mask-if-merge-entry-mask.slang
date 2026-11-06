// After an `if` that every lane leaves through its merge block, a wave op must
// receive the mask from before the `if`, not the mask of the lanes that took
// the same side.
//
// Lanes 0-19 load a nonzero value and lanes 20-31 keep `w = 0`, so every lane
// must see the same 32-lane ballot, 0x000FFFFF. On CUDA the kernel needs no
// ballot besides its initial mask: the region has no exit other than its
// merge block, so the mask after it is the mask on entry.

//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute -line-directive-mode none -validate-ir
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-vk -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE(filecheck-buffer=CHECK):-cuda -shaderobj -output-using-type
//TEST_INPUT: set inp = ubuffer(data=[1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32], stride=4)
//TEST_INPUT: set outp = out ubuffer(data=[0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0], stride=4)

StructuredBuffer<uint> inp;
RWStructuredBuffer<uint> outp;

[shader("compute")]
[numthreads(32, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint w = 0;
    if (tid.x < 20)
        w = inp[tid.x];
    outp[tid.x] = WaveActiveBallot(w != 0).x;
}

// CUDA-LABEL: __global__ void computeMain(
// CUDA: __ballot_sync(4294967295U, true)
// CUDA-NOT: __ballot_sync(

// CHECK-COUNT-32: {{^}}1048575{{$}}
