//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#typealignment--the-operand-order-exception
//META: doc_section_digest=1b464e6340173025e21e0129aef42edde338d1c2b441ba933bd965108bf692da
//META: purpose=Stride is not stored at all: a float[4] in a constant buffer strides by 16 because the array's own alignment is 16 even though the element reports size and alignment 4.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the derived-stride rule with the doc's own worked example. Nothing
// in the layout family records a stride, so the only way to show the stride is
// 16 is to read the two numbers it is derived from: the array's `TypeAlignment`
// of 16, and its total `size` of 52 -- three strides of 16 plus a final element
// of 4, which is arithmetic that only holds if the stride is the alignment and
// not the element size. The CHECKs also follow the array record's operand 0 to
// the element layout and require that element to report size 4 and alignment
// 4, so the two levels are shown to disagree exactly as the doc says.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -dump-ir -o /dev/null -entry main -stage compute

cbuffer CB
{
    float arr[4];
}

RWStructuredBuffer<float> gOut;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    gOut[0] = arr[tid.x];
}

// CHECK: = structFieldLayout(%arr, %[[VL:[0-9]+]])
// CHECK: {{^}}let %[[VL]]{{[^0-9]}}{{.*}}= varLayout(%[[ATL:[0-9]+]],
// Operand 0 is the element type layout; the two after it are the array's own
// attribute tail. No operand carries a stride.
// CHECK: {{^}}let %[[ATL]]{{[^0-9]}}{{.*}}= arrayTypeLayout(%[[ELEM:[0-9]+]], %[[ASZ:[0-9]+]], %[[AALIGN:[0-9]+]])
// CHECK: {{^}}let %[[AALIGN]]{{[^0-9]}}{{.*}}= TypeAlignment(16 : Int)
// CHECK: {{^}}let %[[ASZ]]{{[^0-9]}}{{.*}}= size(8 : Int, 52 : Int)
// CHECK: {{^}}let %[[ELEM]]{{[^0-9]}}{{.*}}= typeLayout(%[[ESZ:[0-9]+]], %[[EALIGN:[0-9]+]])
// CHECK: {{^}}let %[[EALIGN]]{{[^0-9]}}{{.*}}= TypeAlignment(4 : Int)
// CHECK: {{^}}let %[[ESZ]]{{[^0-9]}}{{.*}}= size(8 : Int, 4 : Int)
