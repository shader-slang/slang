//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#size-and-offset
//META: doc_section_digest=4c5526c573a01002c5664c82dafecd90b68ec6487726865d6f3eb388eabf2bf3
//META: purpose=A rasterizer-stage input parameter uses LayoutResourceKind 5, varying input, in operand 0 of its size and offset attributes.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies one row of the resource-kind enumeration that operand 0 of `size`
// and `offset` encodes. Varying parameters are the rows a compute entry point
// never produces, so this test uses a fragment stage. The CHECKs walk from the
// named `uv` field key of the parameter struct to its offset and to the size
// on its type layout, requiring 5 in operand 0 of both.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage fragment

struct FsInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
}

float4 main(FsInput inp) : SV_Target
{
    return float4(inp.uv, 0.0, 1.0);
}

// CHECK: = structFieldLayout(%uv, %[[VL:[0-9]+]])
// CHECK: {{^}}let %[[VL]]{{[^0-9]}}{{.*}}= varLayout(%[[TL:[0-9]+]], %[[OFF:[0-9]+]],
// CHECK: {{^}}let %[[OFF]]{{[^0-9]}}{{.*}}= offset(5 : Int, 0 : Int)
// CHECK: {{^}}let %[[TL]]{{[^0-9]}}{{.*}}= typeLayout(%[[SZ:[0-9]+]])
// CHECK: {{^}}let %[[SZ]]{{[^0-9]}}{{.*}}= size(5 : Int, 1 : Int)
