//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:58:23+00:00
//META: source_commit=57fb63455095efe1465661001ad147b24b968932
//META: doc_ref=docs/language-reference/basics-memory-model-consistency.md#barriers
//META: doc_section_digest=28e72df67f1e8f2f68fd59f23431c2be534ec39fa4206bc57dd04922591e5594
//META: purpose=`*WithGroupSync` barrier variants emit `OpControlBarrier` (thread group execution sync + memory barrier), while their non-sync counterparts emit `OpMemoryBarrier` (memory-only, no execution sync).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc names both barrier variants: e.g. AllMemoryBarrier() and
// AllMemoryBarrierWithGroupSync(). The GroupSync variants add a thread-group
// synchronization point. In SPIRV this distinction maps to:
// - Non-sync: OpMemoryBarrier (memory semantics only, no execution scope)
// - GroupSync: OpControlBarrier (execution scope + memory scope + semantics)
// This test verifies the structural distinction at the SPIRV boundary.

//TEST:SIMPLE(filecheck=MEMCHECK):-target spirv-asm -entry computeMemMain -stage compute
//TEST:SIMPLE(filecheck=CTRLCHECK):-target spirv-asm -entry computeCtrlMain -stage compute

RWStructuredBuffer<uint> buf;

[numthreads(64,1,1)]
void computeMemMain(uint3 tid : SV_DispatchThreadID)
{
    buf[tid.x] = tid.x;
    AllMemoryBarrier();         // memory-only: OpMemoryBarrier
    buf[tid.x + 64] = buf[0];
}

[numthreads(64,1,1)]
void computeCtrlMain(uint3 tid : SV_DispatchThreadID)
{
    buf[tid.x] = tid.x;
    AllMemoryBarrierWithGroupSync(); // execution+memory: OpControlBarrier
    buf[tid.x + 64] = buf[0];
}

// Non-sync variant emits OpMemoryBarrier (not OpControlBarrier).
// MEMCHECK: OpMemoryBarrier
// MEMCHECK-NOT: OpControlBarrier

// GroupSync variant emits OpControlBarrier (not just OpMemoryBarrier).
// CTRLCHECK: OpControlBarrier
// CTRLCHECK-NOT: OpMemoryBarrier
