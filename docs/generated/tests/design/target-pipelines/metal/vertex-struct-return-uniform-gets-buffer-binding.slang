//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#legalizeirformetal
//META: doc_section_digest=1b40e6975a461c5c97ead4202c2a70e579c2e6e882749ceb49d57be34bac586c
//META: purpose=`legalizeShaderOutputParamsForMetal` re-targets entry-point-param decorations onto its wrapper, so a hoisted `uniform` vertex parameter still binds as a `constant*` argument with a `[[buffer(0)]]` slot.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `retargetEntryPointParamDecorations` step. A struct-
// returning vertex entry point is replaced by a wrapper function, and the
// `uniform` parameter hoisted in Phase A carries a decoration naming the
// *original* function; without re-pointing it, the later global-context
// pass would drop the uniform and the kernel would read uninitialized
// memory. CHECK asserts the emitted vertex function is the renamed
// `main_0` and still has a `constant`-address-space parameter with a
// `buffer(0)` slot, which is only true if the decoration survived.

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
