//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#size-and-offset
//META: doc_section_digest=4c5526c573a01002c5664c82dafecd90b68ec6487726865d6f3eb388eabf2bf3
//META: purpose=A SamplerState parameter uses LayoutResourceKind 7, sampler state, in operand 0 of its size and offset attributes.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies one row of the resource-kind enumeration that operand 0 of `size`
// and `offset` encodes. A sampler binds on its own kind rather than sharing
// the texture's, which only a per-row test can show: the CHECKs walk from the
// named `gSamp` field key to its offset and to the size on its type layout
// and require 7 in operand 0 of both.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage compute

Texture2D<float4> gTex;
SamplerState gSamp;
RWStructuredBuffer<float> gOut;

[numthreads(1, 1, 1)]
void main()
{
    gOut[0] = gTex.SampleLevel(gSamp, float2(0.25, 0.5), 0.0).z;
}

// CHECK: = structFieldLayout(%gSamp, %[[VL:[0-9]+]])
// CHECK: {{^}}let %[[VL]]{{[^0-9]}}{{.*}}= varLayout(%[[TL:[0-9]+]], %[[OFF:[0-9]+]])
// CHECK: {{^}}let %[[OFF]]{{[^0-9]}}{{.*}}= offset(7 : Int, 0 : Int)
// CHECK: {{^}}let %[[TL]]{{[^0-9]}}{{.*}}= typeLayout(%[[SZ:[0-9]+]])
// CHECK: {{^}}let %[[SZ]]{{[^0-9]}}{{.*}}= size(7 : Int, 1 : Int)
