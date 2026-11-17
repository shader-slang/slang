//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/decorations.md#interpolation-and-shader-io
//META: doc_section_digest=cab069de9f491d651e18ca65a976f70486f9bac662a529f4565a5ed9fa18a2a4
//META: purpose=A vertex-shader parameter declared with a non-SV semantic (POSITION) carries the same semantic decoration shape as SV-prefixed semantics, with index -1 when none is given.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `semantic` encoding is uniform across user-defined and
// system-value semantics - the decoration does not switch shape for the
// `SV_` family. The CHECK pins `("POSITION", -1 : Int)`, the same operand
// pair the `SV_DispatchThreadID` sibling pins.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o - -entry main -stage vertex

[shader("vertex")]
float4 main(float3 pos : POSITION) : SV_Position
{
    return float4(pos, 1.0);
}

// CHECK: [semantic("POSITION", -1 : Int)]
