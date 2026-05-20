//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:13:48Z
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/llm-generated/ir-reference/decorations.md#interpolation-and-shader-io
//META: doc_section_digest=4430d3efb9516cd383356961351a223b29ad1c948e7c5ea9bf0a36d832a4b940
//META: purpose=An [earlydepthstencil] attribute lowers to a bare earlyDepthStencil decoration with no operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc records earlyDepthStencil ("Requests early-depth-stencil
// test for a pixel shader") with no operands and AST origin
// "[earlydepthstencil] attribute". Although the doc situates it on
// a pixel shader, the decoration attaches whenever the modifier is
// present on a function inst; we observe it here on a compute
// entry for IR-dump observability (the decoration survives
// LOWER-TO-IR regardless of stage selection).

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
[earlydepthstencil]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [earlyDepthStencil]
