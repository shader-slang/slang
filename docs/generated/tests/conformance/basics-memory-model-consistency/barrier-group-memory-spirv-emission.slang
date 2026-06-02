//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:23+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-consistency.md#barriers
//META: doc_section_digest=28e72df67f1e8f2f68fd59f23431c2be534ec39fa4206bc57dd04922591e5594
//META: purpose=`GroupMemoryBarrier()` emits `OpMemoryBarrier` with Workgroup scope and WorkgroupMemory semantics in SPIRV, covering only thread-group (shared) memory accesses.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `GroupMemoryBarrier()` emits `OpMemoryBarrier` with Workgroup scope (2)
// and semantics 264 (SeqCst | WorkgroupMemory), covering only groupshared memory.
// CHECK-SAME chains pin %uint_2 (Workgroup scope) and %uint_264; the absence of
// UniformMemory and ImageMemory bits distinguishes this from device-memory barriers.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSLCHECK):-target hlsl -entry computeMain -stage compute

groupshared uint sharedBuf[64];
RWStructuredBuffer<uint> buf;

[numthreads(64,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID, uint3 gid : SV_GroupThreadID)
{
    sharedBuf[gid.x] = tid.x;
    GroupMemoryBarrier();
    buf[tid.x] = sharedBuf[63 - gid.x];
}

// SPIRV: OpMemoryBarrier Workgroup scope (2), thread-group semantics (264).
// CHECK: OpMemoryBarrier
// CHECK-SAME: %uint_2
// CHECK-SAME: %uint_264

// HLSL: preserves GroupMemoryBarrier intrinsic name.
// HLSLCHECK: GroupMemoryBarrier()
