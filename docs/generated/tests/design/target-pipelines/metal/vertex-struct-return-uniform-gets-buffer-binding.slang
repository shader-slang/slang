//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:00Z
//META: source_commit=b15b8060d2525b8cd123f2268fafbcd82e837fb3
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#legalizeirformetal
//META: doc_section_digest=43faa16ce410de3ffb63136a8e3d7ca1d85bcf4dd72e95e139b70b0b674d35a6
//META: purpose=For a struct-returning vertex shader, the wrapper built by legalizeVertexShaderOutputParamsForMetal retargets entry-point-param decorations, so a hoisted `uniform` entry-point parameter still binds as a `constant*` parameter with a `[[buffer(0)]]` attribute on the emitted vertex function.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the vertex-shader wrapper swap does not lose the binding of a
// uniform entry-point parameter: the entry returns a struct (forcing the
// wrapper path) and computes its position from the uniform (so the
// parameter is live). The CHECKs assert the emitted vertex function is
// the renamed main_0 and that its parameter list still carries a
// buffer(0)-attributed constant-pointer argument -- the observable
// consequence of retargetEntryPointParamDecorations re-pointing the
// decoration at the wrapper.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage vertex

struct VOut
{
    float4 pos : SV_Position;
}

[shader("vertex")]
VOut main(uint vid : SV_VertexID, uniform float2 scaleBias)
{
    VOut o;
    o.pos = float4(float(vid) * scaleBias.x + scaleBias.y, 0.0, 0.0, 1.0);
    return o;
}

// CHECK: position
// CHECK: vertex
// CHECK-SAME: main_{{[0-9]+}}
// CHECK-SAME: constant
// CHECK-SAME: buffer(0)
