// A behavioral net for the CPU (C++-source) half of the reconstruction
// gate: the guard idiom runs end-to-end on -cpu with dispatch threads
// beyond the bound, pinning correct results for a reconstructed chain.
// The deterministic value is value-correctness against chain-logic
// miscompiles (arm swap, polarity) on the one gated target runnable in
// GPU-less CI; a wrongly-hoisted out-of-bounds read is caught only
// non-deterministically, since the guard masks the loaded value.
// (Companion to shader-slang/slang#14494.)

//TEST:COMPARE_COMPUTE(filecheck-buffer=CHECK):-cpu -output-using-type

//TEST_INPUT: set input = ubuffer(data=[3 5 0 0 0 0 0 0], stride=4)
//TEST_INPUT: set output = out ubuffer(data=[0 0 0 0 0 0 0 0], stride=4)

RWStructuredBuffer<uint> input;
RWStructuredBuffer<uint> output;

[shader("compute")]
[numthreads(8, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint count = input[0];
    if (tid.x < count && input[tid.x] > 0)
    {
        output[tid.x] = 1;
    }
    else
    {
        output[tid.x] = 0;
    }
    //CHECK: type: uint32_t
    //CHECK-NEXT: 1
    //CHECK-NEXT: 1
    //CHECK-NEXT: 0
    //CHECK-NEXT: 0
    //CHECK-NEXT: 0
    //CHECK-NEXT: 0
    //CHECK-NEXT: 0
    //CHECK-NEXT: 0
}
