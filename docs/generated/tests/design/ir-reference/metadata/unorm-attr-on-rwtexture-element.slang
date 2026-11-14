//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#attr-family
//META: doc_section_digest=d04af1d542b300e2a0bbb935e77e3c2a63823444c7ee62a2765c374e4959f38b
//META: purpose=The unorm modifier on a texture element type produces a no-operand unorm attribute inside an Attributed type wrapper.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the UNORM marker attribute. `RWTexture2D<unorm float4>` is the
// user-level surface; the CHECK captures the id of the attribute definition
// (which prints with no operand list at all, matching a zero-operand
// attribute) and then requires that same id to appear as the second operand of
// an `Attributed` type -- the wrapper the doc says carries it.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o - -entry main -stage compute

RWTexture2D<unorm float4> tex;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    tex[tid.xy] = float4(0.5);
}

// CHECK: %[[U:[0-9]+]]{{.*}}= unorm
// CHECK: Attributed({{.*}}, %[[U]])
