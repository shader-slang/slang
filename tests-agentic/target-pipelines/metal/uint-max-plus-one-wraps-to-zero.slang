//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/llm-generated/target-pipelines/metal.md#phase-d-metal-emit-and-downstream-tools
//META: doc_section_digest=15e31694fd5ee22d184f1b99f92912686150441a720712b25e415c394b7ce97f
//META: purpose=Boundary: literal `uint(0xFFFFFFFF) + 1u` is constant-folded by the Metal pipeline to `0U` in the emitted text, demonstrating wrap-on-overflow at the unsigned MAX edge.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: unsigned MAX wrap. With both operands as
// literals, IR-level constant folding collapses the addition
// before emit, and the wrap result `0U` shows up directly in the
// Metal text. This pins the documented unsigned-wrap semantics.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

RWStructuredBuffer<uint> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    uint mx = 0xFFFFFFFFu;
    outBuf[tid.x] = mx + 1u;
}

// CHECK: 0U
