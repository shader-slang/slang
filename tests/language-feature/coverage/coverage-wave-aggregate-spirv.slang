// Documents and pins the wave-aggregation structure that makes aggregated
// coverage counts equal to the per-lane baseline (issue #11509).
//
// Per marker the coverage pass emits, on wave-capable targets:
//     uint lc = WaveActiveCountBits(true);   // active lanes at this marker
//     if (WaveIsFirstLane()) atomicAdd(slot, lc);
// so each marker performs exactly ONE atomic whose addend is the active-lane
// count. Summed over all wave executions of a marker this equals the per-lane
// total (every active lane would otherwise have added 1), i.e. counts are
// equal to the per-lane baseline BY CONSTRUCTION.
//
// This is a static, structural check. Runtime multi-lane verification (that a
// real GPU's reconvergence makes the aggregated total bit-identical to the
// per-lane total under divergent control flow) needs a GPU and is not possible
// in CI; the subsystem owner validates that via the image-pipeline demo.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage compute -entry computeMain -trace-coverage

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

// The body is two straight-line statements, so under line `-trace-coverage`
// it produces exactly two line markers → coverage slots %int_0 and %int_1, and
// no %int_2. The slot-0 / slot-1 CHECKs and the final %int_2 CHECK-NOT below
// depend on that mapping; editing the body (adding/removing a statement)
// changes the slot count and would need the checks updated to match.
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = tid.x;
    outputBuffer[1] = tid.y;
}

// The aggregated form needs the subgroup ballot capability.
//CHECK: OpCapability GroupNonUniformBallot

// First marker (slot 0): the atomic adds the active-lane COUNT (the
// BallotBitCount result), not a literal 1, and only the elected first lane
// reaches the atomic. Bind the count's result id and confirm it is the value
// operand of the guarded atomic — this is what makes the aggregated total equal
// the per-lane total.
//CHECK: OpAccessChain {{.*}} %__slang_coverage %int_0 %int_0
//CHECK: %[[LC0:[0-9]+]] = OpGroupNonUniformBallotBitCount %uint %uint_3 Reduce %{{[0-9]+}}
//CHECK: OpGroupNonUniformElect %bool %uint_3
//CHECK: OpBranchConditional
//CHECK: OpAtomicIAdd %uint %{{[0-9]+}} %uint_1 %uint_0 %[[LC0]]

// Second marker (slot 1): same aggregated shape on a distinct slot, and no
// third slot is synthesized (exactly two markers).
//CHECK: OpAccessChain {{.*}} %__slang_coverage %int_0 %int_1
//CHECK: OpGroupNonUniformBallotBitCount
//CHECK-NOT: OpAccessChain {{.*}} %__slang_coverage %int_0 %int_2
