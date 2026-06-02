//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:23+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-consistency.md#barriers
//META: doc_section_digest=28e72df67f1e8f2f68fd59f23431c2be534ec39fa4206bc57dd04922591e5594
//META: purpose=The three address-space scopes (All, Device, ThreadGroup) for memory barriers produce distinct SPIRV memory semantics operands: All=2376, Device=2120, ThreadGroup=264.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc defines three distinct address-space scopes for barriers:
// - All: device + thread group memory accesses (semantics 2376)
// - Device: address spaces with all-threads instance scope, including storage buffers (semantics 2120)
// - Thread group: thread group (groupshared) memory accesses (semantics 264)
// This test verifies the three scopes produce non-overlapping SPIRV semantics values,
// confirming the boundary between the scope definitions.

//TEST:SIMPLE(filecheck=ALLCHECK):-target spirv-asm -entry computeAllMain -stage compute
//TEST:SIMPLE(filecheck=DEVCHECK):-target spirv-asm -entry computeDevMain -stage compute
//TEST:SIMPLE(filecheck=GRPCHECK):-target spirv-asm -entry computeGrpMain -stage compute

RWStructuredBuffer<uint> buf;
groupshared uint shared1[64];

[numthreads(64,1,1)]
void computeAllMain(uint3 tid : SV_DispatchThreadID, uint3 gid : SV_GroupThreadID)
{
    buf[tid.x] = tid.x;
    AllMemoryBarrier();
    buf[tid.x + 1] = buf[0];
}

[numthreads(64,1,1)]
void computeDevMain(uint3 tid : SV_DispatchThreadID)
{
    buf[tid.x] = tid.x;
    DeviceMemoryBarrier();
    buf[tid.x + 1] = buf[0];
}

[numthreads(64,1,1)]
void computeGrpMain(uint3 tid : SV_DispatchThreadID, uint3 gid : SV_GroupThreadID)
{
    shared1[gid.x] = tid.x;
    GroupMemoryBarrier();
    buf[tid.x] = shared1[63 - gid.x];
}

// All: semantics 2376 = SeqCst|UniformMemory|WorkgroupMemory|ImageMemory
// ALLCHECK: OpMemoryBarrier
// ALLCHECK-SAME: %uint_2376

// Device: semantics 2120 = SeqCst|UniformMemory|ImageMemory (no WorkgroupMemory)
// DEVCHECK: OpMemoryBarrier
// DEVCHECK-SAME: %uint_2120

// ThreadGroup: semantics 264 = SeqCst|WorkgroupMemory (no UniformMemory or ImageMemory)
// GRPCHECK: OpMemoryBarrier
// GRPCHECK-SAME: %uint_264
