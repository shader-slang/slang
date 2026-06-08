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

// Counter increments are wave-aggregated (issue #11509): each marker counts
// the active lanes (WaveActiveCountBits) and a single lane (WaveIsFirstLane)
// applies the whole increment, instead of every lane issuing atomicAdd(slot, 1).
//CHECK-DAG: WaveActiveCountBits
//CHECK-DAG: WaveIsFirstLane
// One distinct counter slot per marker (no slot dedup by file/line).
//CHECK-DAG: _slang_coverage{{.*}}int(0)
//CHECK-DAG: _slang_coverage{{.*}}int(1)
//CHECK-DAG: _slang_coverage{{.*}}int(2)
// Each marker still performs a real (now guarded) CUDA atomicAdd on the buffer.
//CHECK-DAG: atomicAdd
//CHECK-DAG: atomicAdd
//CHECK-DAG: atomicAdd
