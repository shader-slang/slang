//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:23+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-consistency.md#general-programming-guidance
//META: doc_section_digest=3d7e3e029e78610815c5af6fd77095dcd95914a8f6598bf9a2d7b55d6080810e
//META: purpose=The doc's recommended pattern (elect first lane with `WaveIsFirstLane` + `AllMemoryBarrierWithWaveSync` + `Atomic<T>.store`) compiles to SPIRV `OpGroupNonUniformElect` + `OpControlBarrier` + `OpAtomicStore`.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc recommends: use AllMemoryBarrierWithWaveSync() to ensure writes complete,
// then elect the first lane (WaveIsFirstLane) to perform a single atomic store,
// reducing the number of per-thread atomic memory accesses.
// This is the code example from the "Minimize the Number of Atomic Memory Accesses"
// section. Verify the pattern compiles and emits the expected SPIRV ops.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSLCHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSLCHECK):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METALCHECK):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDACHECK):-target cuda -entry computeMain -stage compute

RWStructuredBuffer<uint> outputBuffer;
RWStructuredBuffer<Atomic<uint>> syncBuffer;

[numthreads(64,1,1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[dispatchThreadID.x] = dispatchThreadID.x;
    AllMemoryBarrierWithWaveSync();
    if (WaveIsFirstLane())
    {
        syncBuffer[0].store(1, MemoryOrder.Relaxed);
    }
}

// SPIRV: wave-scope ControlBarrier (Subgroup 3), then OpGroupNonUniformElect for WaveIsFirstLane,
// then OpAtomicStore for the Relaxed store. OpControlBarrier appears before OpGroupNonUniformElect
// in program order; use CHECK-DAG for unordered matching across these ops.
// CHECK-DAG: OpControlBarrier
// CHECK-DAG: OpGroupNonUniformElect
// CHECK-DAG: OpAtomicStore

// HLSL: emits WaveIsFirstLane() and AllMemoryBarrier (wave-sync not expressible in HLSL).
// HLSLCHECK: AllMemoryBarrier
// HLSLCHECK: WaveIsFirstLane

// GLSL: emits subgroupBarrier (wave-sync), subgroupElect (first-lane), and a plain store for Relaxed Atomic.
// GLSLCHECK-DAG: subgroupBarrier
// GLSLCHECK-DAG: subgroupElect

// Metal: emits simdgroup_barrier (wave-sync), simd_is_first (first-lane), and atomic_store_explicit (Relaxed store).
// METALCHECK-DAG: simdgroup_barrier
// METALCHECK-DAG: simd_is_first
// METALCHECK-DAG: atomic_store_explicit

// CUDA: emits __syncwarp (wave-sync), WaveIsFirstLane helper (first-lane), and plain pointer write for Relaxed store.
// CUDACHECK-DAG: __syncwarp
// CUDACHECK-DAG: WaveIsFirstLane

// Targets not covered by emission directives:
//   wgsl   - AllMemoryBarrierWithWaveSync is not available on wgsl target (E36107 capability gate; no wave-sync surface in WGSL).
//   cpp    - AllMemoryBarrierWithWaveSync is not available on cpp target (E36107 capability gate).
