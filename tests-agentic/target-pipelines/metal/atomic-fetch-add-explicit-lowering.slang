//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#phase-c-metal-legalization-lowering-phi-elimination
//META: doc_section_digest=de6a2b24718ef9344624fa3cfc660b5e13c913b28787d2b06b2db1085587ae5e
//META: purpose=InterlockedAdd lowers to Metal's atomic_fetch_add_explicit with memory_order_relaxed and an (atomic_uint device*) cast.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `validateAtomicOperations` runs for Metal. The Metal emitter
// renders HLSL-style `InterlockedAdd(buf[0], 1, prev)` as
// `atomic_fetch_add_explicit((atomic_uint device*)(buf+0), 1,
// memory_order_relaxed)`.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<uint> counter;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint prev;
    InterlockedAdd(counter[0], 1u, prev);
}

// CHECK: atomic_fetch_add_explicit
// CHECK-SAME: atomic_uint device*
// CHECK-SAME: memory_order_relaxed
