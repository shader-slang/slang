//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#interpolation-and-shader-io
//META: doc_section_digest=53ad1d566b89727f915f7a684edad14e039800bbbb1330d913ea0c67195afd17
//META: purpose=An [earlydepthstencil] attribute lowers to a bare earlyDepthStencil decoration with no operands.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `earlyDepthStencil` row's operand shape: the decoration is
// nullary, so the dump prints `[earlyDepthStencil]` with no parentheses.
// The attribute is written on a compute entry point because the decoration
// is attached from the modifier during lowering independently of the stage
// the entry point is compiled for, which keeps the observation on a shader
// that needs no rasterizer plumbing.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
[earlydepthstencil]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = int(tid.x);
}

// CHECK: [earlyDepthStencil]
