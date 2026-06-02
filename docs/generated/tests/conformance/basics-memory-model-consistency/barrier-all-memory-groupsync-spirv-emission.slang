//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:23+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-consistency.md#barriers
//META: doc_section_digest=28e72df67f1e8f2f68fd59f23431c2be534ec39fa4206bc57dd04922591e5594
//META: purpose=`AllMemoryBarrierWithGroupSync()` emits `OpControlBarrier` with Workgroup execution scope plus Device memory scope and all-memory semantics in SPIRV.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists AllMemoryBarrierWithGroupSync() as covering "All" address space scope
// with a thread-group synchronization point. SPIRV: OpControlBarrier with execution
// scope Workgroup (2), memory scope Device (1), semantics 2376
// (= SeqCst | UniformMemory | WorkgroupMemory | ImageMemory).
// The execution scope Workgroup is what differentiates this from AllMemoryBarrier().

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSLCHECK):-target hlsl -entry computeMain -stage compute

RWStructuredBuffer<uint> buf;

[numthreads(64,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    buf[tid.x] = tid.x;
    AllMemoryBarrierWithGroupSync();
    buf[tid.x + 1] = buf[0];
}

// SPIRV: OpControlBarrier Workgroup(2) Device(1) all-memory-semantics(2376).
// CHECK: OpControlBarrier
// CHECK-SAME: %uint_2
// CHECK-SAME: %uint_1
// CHECK-SAME: %uint_2376

// HLSL: preserves AllMemoryBarrierWithGroupSync intrinsic name.
// HLSLCHECK: AllMemoryBarrierWithGroupSync()
