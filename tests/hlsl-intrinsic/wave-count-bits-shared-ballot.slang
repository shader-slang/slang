// WavePrefixCountBits and WaveActiveCountBits both build their subgroup lane mask with an
// OpGroupNonUniformBallot. When both run on the same predicate in one basic block with nothing
// between them that changes subgroup participation, the two structurally-identical ballots are
// merged so a single OpGroupNonUniformBallot feeds both OpGroupNonUniformBallotBitCount ops
// (computeMerged). Anything that might change participation between them conservatively fences the
// merge, so both ballots survive: a barrier (computeBarrier), a subgroup op that is not on the
// mask-read whitelist (computeOpaqueOp), or even a side-effect-free call whose callee could still
// demote a lane (computeCall). Two ballots on *different* predicates are likewise never merged
// (computeDifferentPredicate), which guards the predicate operand comparison in ballotsAreEquivalent
// against a future weakening that would silently miscompile one of the two counts.

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -stage compute -entry computeMerged -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_BARRIER): -target spirv-asm -stage compute -entry computeBarrier -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_OPAQUE): -target spirv-asm -stage compute -entry computeOpaqueOp -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_CALL): -target spirv-asm -stage compute -entry computeCall -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_DIFFPRED): -target spirv-asm -stage compute -entry computeDifferentPredicate -emit-spirv-directly

RWStructuredBuffer<uint> outputBuffer;

[noinline]
[__readNone]
uint sideEffectFreeDouble(uint x)
{
    return x * 2u;
}

[numthreads(32, 1, 1)]
void computeMerged(uint3 tid : SV_DispatchThreadID)
{
    bool p = (tid.x & 5) != 0;

    // A single ballot is emitted (no private "__mask" register name leaks into OpName), and both
    // bit-counts read that same ballot result -- ExclusiveScan for the prefix count, Reduce for the
    // active count.
    // CHECK-NOT: __mask
    // CHECK: [[BALLOT:%[0-9]+]] = OpGroupNonUniformBallot %v4uint
    // CHECK-NOT: OpGroupNonUniformBallot %v4uint
    // CHECK-DAG: OpGroupNonUniformBallotBitCount %uint {{.*}} ExclusiveScan [[BALLOT]]
    // CHECK-DAG: OpGroupNonUniformBallotBitCount %uint {{.*}} Reduce [[BALLOT]]
    uint prefix = WavePrefixCountBits(p);
    uint active = WaveActiveCountBits(p);
    outputBuffer[tid.x] = prefix + active;
}

[numthreads(32, 1, 1)]
void computeBarrier(uint3 tid : SV_DispatchThreadID)
{
    bool p = (tid.x & 5) != 0;

    // CHECK_BARRIER-COUNT-2: OpGroupNonUniformBallot %v4uint
    uint prefix = WavePrefixCountBits(p);
    GroupMemoryBarrierWithGroupSync();
    uint active = WaveActiveCountBits(p);
    outputBuffer[tid.x] = prefix + active;
}

[numthreads(32, 1, 1)]
void computeOpaqueOp(uint3 tid : SV_DispatchThreadID)
{
    bool p = (tid.x & 5) != 0;

    // CHECK_OPAQUE-COUNT-2: OpGroupNonUniformBallot %v4uint
    uint prefix = WavePrefixCountBits(p);
    uint sum = WaveActiveSum(tid.x);
    uint active = WaveActiveCountBits(p);
    outputBuffer[tid.x] = prefix + active + sum;
}

[numthreads(32, 1, 1)]
void computeCall(uint3 tid : SV_DispatchThreadID)
{
    bool p = (tid.x & 5) != 0;

    // CHECK_CALL-COUNT-2: OpGroupNonUniformBallot %v4uint
    uint prefix = WavePrefixCountBits(p);
    uint m = sideEffectFreeDouble(tid.x);
    uint active = WaveActiveCountBits(p);
    outputBuffer[tid.x] = prefix + active + m;
}

[numthreads(32, 1, 1)]
void computeDifferentPredicate(uint3 tid : SV_DispatchThreadID)
{
    bool p = (tid.x & 5) != 0;
    bool q = (tid.x & 2) != 0;

    // Distinct predicates ballot distinct lane masks, so ballotsAreEquivalent must keep them apart:
    // both ballots survive.
    // CHECK_DIFFPRED-COUNT-2: OpGroupNonUniformBallot %v4uint
    uint prefix = WavePrefixCountBits(p);
    uint active = WaveActiveCountBits(q);
    outputBuffer[tid.x] = prefix + active;
}
