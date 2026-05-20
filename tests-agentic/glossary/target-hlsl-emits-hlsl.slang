//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/glossary.md#terms
//META: doc_section_digest=c590da2fa78de86b023f0aa1d0df2f46a79bb2de597705bc7ed2fee90af2869b
//META: purpose=target: a `TargetRequest` selects a code-generation format; passing `-target hlsl` produces HLSL text containing the entry-point function.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "target" glossary entry: with `-target hlsl` the
// emitted artifact is HLSL source text, and the compute entry
// point `main` appears in it.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    // CHECK: void main
    buf[0] = tid.x + 1;
}
