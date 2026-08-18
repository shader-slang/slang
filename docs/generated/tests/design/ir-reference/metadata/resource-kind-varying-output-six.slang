//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#size-and-offset
//META: doc_section_digest=4c5526c573a01002c5664c82dafecd90b68ec6487726865d6f3eb388eabf2bf3
//META: purpose=A rasterizer-stage result uses LayoutResourceKind 6, varying output, in operand 0 of its size and offset attributes.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies one row of the resource-kind enumeration that operand 0 of `size`
// and `offset` encodes. The output row is reached through the entry point's
// result layout rather than through a parameter, so the CHECKs enter at the
// `EntryPointLayout` record, take its second operand, and require 6 in
// operand 0 of the offset and size hanging off that var layout.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage fragment

float4 main(float2 uv : TEXCOORD0) : SV_Target
{
    return float4(uv.y, uv.x, 0.5, 1.0);
}

// CHECK: = EntryPointLayout(%{{[0-9]+}}, %[[RES:[0-9]+]])
// CHECK: {{^}}let %[[RES]]{{[^0-9]}}{{.*}}= varLayout(%[[TL:[0-9]+]], %[[OFF:[0-9]+]],
// CHECK: {{^}}let %[[OFF]]{{[^0-9]}}{{.*}}= offset(6 : Int, 0 : Int)
// CHECK: {{^}}let %[[TL]]{{[^0-9]}}{{.*}}= typeLayout(%[[SZ:[0-9]+]])
// CHECK: {{^}}let %[[SZ]]{{[^0-9]}}{{.*}}= size(6 : Int, 1 : Int)
