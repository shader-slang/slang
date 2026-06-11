// Anti-regression for issue #11509 — a NON-coverage compile must NOT pull in
// the force-kept wave intrinsics (WaveActiveCountBits/WaveIsFirstLane) and must
// synthesize no __slang_coverage buffer. Guards the linker's
// keepCoverageWaveFuncs no-leak invariant: if a future edit weakened the
// coverage/aggregation gate, these subgroup ops + the GroupNonUniformBallot
// capability would start appearing in EVERY SPIR-V/CUDA/Metal/HLSL>=SM6.0
// shader. No -trace-* flag is passed here, so the aggregated path must be inert.

//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage compute -entry computeMain

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = tid.x;
}

// A positive anchor so FileCheck has a match, then the no-leak NOTs.
//SPV: OpEntryPoint

// No wave intrinsics leaked in: the coverage force-keep is gated off here.
//SPV-NOT: OpGroupNonUniformBallotBitCount
//SPV-NOT: OpGroupNonUniformElect

// And no coverage buffer was synthesized.
//SPV-NOT: __slang_coverage
