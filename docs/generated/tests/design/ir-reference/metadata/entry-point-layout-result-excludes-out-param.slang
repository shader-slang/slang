//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#varlayout-and-entrypointlayout
//META: doc_section_digest=0ce1b4071a5a571cfc1945ea83ae995cc4721673793ce1d486650e97ce5658bb
//META: purpose=An EntryPointLayout's result layout covers only the return value, so an out parameter is laid out in the parameters layout instead.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out that makes the second operand's name misleading if
// read as "everything the entry point writes". The entry point writes two
// things: a returned colour and an `out` depth. The CHECKs take operand 1 of
// the layout record and require its semantic to be the return value's
// `SV_Target`, then take operand 0 and find the `out` parameter's `SV_Depth`
// among the fields of the parameter struct layout -- so the `out` parameter is
// demonstrably on the parameters side, not the result side.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage fragment

float4 main(float2 uv : TEXCOORD0, out float depth : SV_Depth) : SV_Target
{
    depth = uv.x;
    return float4(uv, 0.0, 1.0);
}

// CHECK: = EntryPointLayout(%[[PARAMS:[0-9]+]], %[[RESULT:[0-9]+]])
// The result layout carries the returned value's semantic and nothing else.
// CHECK: {{^}}let %[[RESULT]]{{[^0-9]}}{{.*}}= varLayout(%{{[0-9]+}},
// CHECK: = systemValueSemantic("SV_Target", 0 : Int)
// CHECK: {{^}}let %[[PARAMS]]{{[^0-9]}}{{.*}}= varLayout(%[[PSTRUCT:[0-9]+]])
// CHECK: {{^}}let %[[PSTRUCT]]{{[^0-9]}}{{.*}}= structTypeLayout(
// The `out` parameter's semantic appears under the parameters layout.
// CHECK: = systemValueSemantic("SV_Depth", 0 : Int)
