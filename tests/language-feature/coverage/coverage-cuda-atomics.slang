// Verifies that -trace-coverage on a CUDA target emits native
// `atomicAdd` instructions on the synthesized counter buffer.
// Mirror of coverage-spirv-atomics.slang for the CUDA backend:
// same shape, but CUDA's binding model is different (no descriptor
// sets — a module-scope `__constant__ GlobalParams` that the host
// binds by symbol lookup), so validating that coverage flows through
// correctly on CUDA exercises an independent code path.
//
// Dispatch validation is separate — this test just covers the
// codegen invariants that matter for a CUDA integration.

//TEST:SIMPLE(filecheck=CHECK):-target cuda -stage compute -entry computeMain -trace-coverage

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint accum = 0;
    for (uint i = 0; i < 4; ++i)
    {
        if ((i & 1u) == 0u)
            accum += i;
        else
            accum += i * 2u;
    }
    outputBuffer[0] = accum;
}

// The synthesized buffer must land in GlobalParams (reflection-visible)
// so the host can bind it via the standard CUDA module-symbol path.
//CHECK: struct GlobalParams
//CHECK: RWStructuredBuffer{{.*}}_slang_coverage

// Counter increments are wave-aggregated (issue #11509): each marker counts the
// active lanes and a single elected lane applies the whole increment, instead of
// every lane issuing atomicAdd(slot, 1). On CUDA `WaveActiveCountBits` lowers to
// `__popc(__ballot_sync(...))`, so assert that real idiom (not just the kept
// function name), then bind each guarded `atomicAdd` to BOTH its coverage-slot
// pointer and the active-lane-count result so the test proves the atomic targets
// the slot and adds the lane count, not a literal 1.
//CHECK: __popc(__ballot_sync
// slot 0: capture the slot pointer and the lane count, then bind the atomic.
//CHECK: [[SLOT0:_S[0-9]+]] = {{.*}}_slang_coverage{{.*}}int(0)
//CHECK: [[CNT0:_S[0-9]+]] = WaveActiveCountBits
//CHECK: atomicAdd([[SLOT0]], [[CNT0]])
// slot 1
//CHECK: [[SLOT1:_S[0-9]+]] = {{.*}}_slang_coverage{{.*}}int(1)
//CHECK: [[CNT1:_S[0-9]+]] = WaveActiveCountBits
//CHECK: atomicAdd([[SLOT1]], [[CNT1]])
// slot 2
//CHECK: [[SLOT2:_S[0-9]+]] = {{.*}}_slang_coverage{{.*}}int(2)
//CHECK: [[CNT2:_S[0-9]+]] = WaveActiveCountBits
//CHECK: atomicAdd([[SLOT2]], [[CNT2]])
