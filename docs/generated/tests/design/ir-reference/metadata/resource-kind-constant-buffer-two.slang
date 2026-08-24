//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#size-and-offset
//META: doc_section_digest=4c5526c573a01002c5664c82dafecd90b68ec6487726865d6f3eb388eabf2bf3
//META: purpose=A cbuffer's own binding uses LayoutResourceKind 2, constant buffer, in operand 0 of its size and offset attributes.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies one row of the resource-kind enumeration that operand 0 of `size`
// and `offset` encodes. Neither operand prints as anything but a bare integer,
// so a reader can only decode a dump by knowing which construct produces which
// number -- and a test per row is what keeps that mapping honest. The CHECKs
// walk from the named `CB` field key to its container var layout and require
// the kind there to be 2 on both the size and the offset side.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage compute

cbuffer CB
{
    float4 tint;
}

RWStructuredBuffer<float> gOut;

[numthreads(1, 1, 1)]
void main()
{
    gOut[0] = tint.w;
}

// CHECK: = structFieldLayout(%CB, %[[VL:[0-9]+]])
// CHECK: {{^}}let %[[VL]]{{[^0-9]}}{{.*}}= varLayout(%[[PGTL:[0-9]+]], %[[OFF:[0-9]+]])
// CHECK: {{^}}let %[[PGTL]]{{[^0-9]}}{{.*}}= parameterGroupTypeLayout(%[[CONTAINER:[0-9]+]],
// The container var layout shares the very same `offset` instruction, which is
// the hoisting the layout family relies on.
// CHECK: {{^}}let %[[CONTAINER]]{{[^0-9]}}{{.*}}= varLayout(%[[TL:[0-9]+]], %[[OFF]])
// CHECK: {{^}}let %[[OFF]]{{[^0-9]}}{{.*}}= offset(2 : Int, 0 : Int)
// CHECK: {{^}}let %[[TL]]{{[^0-9]}}{{.*}}= typeLayout(%[[SZ:[0-9]+]])
// CHECK: {{^}}let %[[SZ]]{{[^0-9]}}{{.*}}= size(2 : Int, 1 : Int)
