//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:23+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-consistency.md#barriers
//META: doc_section_digest=28e72df67f1e8f2f68fd59f23431c2be534ec39fa4206bc57dd04922591e5594
//META: purpose=`GroupMemoryBarrierWithGroupSync()` emits `OpControlBarrier` with Workgroup scope for both execution and memory, covering only thread-group memory with a synchronization point.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists GroupMemoryBarrierWithGroupSync() as covering "Thread group" scope
// with a thread-group synchronization point.
// SPIRV: OpControlBarrier with execution scope Workgroup (2), memory scope Workgroup (2),
// semantics 264 (= SeqCst(8) | WorkgroupMemory(256)). Both execution and memory scopes
// are Workgroup — distinguishing it from DeviceMemoryBarrierWithGroupSync.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=HLSLCHECK):-target hlsl -entry computeMain -stage compute

groupshared uint sharedBuf[64];
RWStructuredBuffer<uint> buf;

[numthreads(64,1,1)]
void computeMain(uint3 tid : SV_DispatchThreadID, uint3 gid : SV_GroupThreadID)
{
    sharedBuf[gid.x] = tid.x;
    GroupMemoryBarrierWithGroupSync();
    buf[tid.x] = sharedBuf[63 - gid.x];
}

// SPIRV: OpControlBarrier Workgroup(2) Workgroup(2) thread-group-semantics(264).
// Both execution and memory scope are Workgroup (2).
// CHECK: OpControlBarrier
// CHECK-SAME: %uint_2
// CHECK-SAME: %uint_2
// CHECK-SAME: %uint_264

// HLSL: preserves GroupMemoryBarrierWithGroupSync intrinsic name.
// HLSLCHECK: GroupMemoryBarrierWithGroupSync()
