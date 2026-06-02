//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:23+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-consistency.md#barriers
//META: doc_section_digest=28e72df67f1e8f2f68fd59f23431c2be534ec39fa4206bc57dd04922591e5594
//META: purpose=`AllMemoryBarrier()` emits `OpMemoryBarrier` covering All address spaces (device + thread group) in SPIRV, with Device scope and SeqCst semantics including Uniform + Workgroup + Image memory.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc states: "A memory barrier imposes reordering constraints for memory
// accesses. Memory accesses before and after a barrier cannot be reordered
// across the barrier, matching the acquire-release memory order for the barrier."
// AllMemoryBarrier covers "all" scope: device and thread group memory accesses.
// SPIRV: OpMemoryBarrier with Device scope (1) and semantics 2376
// (= SequentiallyConsistent(8) | UniformMemory(64) | WorkgroupMemory(256) | ImageMemory(2048)).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSLCHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<uint> buf;

[numthreads(64,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    buf[tid.x] = tid.x;
    AllMemoryBarrier();
    buf[tid.x + 1] = buf[0];
}

// SPIRV: OpMemoryBarrier covers Device scope (1), all memory semantics (2376).
// CHECK: OpMemoryBarrier
// CHECK-SAME: %uint_1
// CHECK-SAME: %uint_2376

// HLSL: preserves AllMemoryBarrier intrinsic name.
// HLSLCHECK: AllMemoryBarrier()
