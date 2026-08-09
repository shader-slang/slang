//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/metadata.md#typealignment--the-operand-order-exception
//META: doc_section_digest=1b464e6340173025e21e0129aef42edde338d1c2b441ba933bd965108bf692da
//META: purpose=A type layout emits every size attribute before any TypeAlignment attribute, so each attribute kind forms one contiguous run of operands.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the contiguity invariant that attribute enumeration depends on.
// A parameter block whose element struct has both byte-unit data and resources
// produces a struct layout with two size attributes and one alignment
// attribute. The CHECK captures the first three operands positionally and then
// resolves them: interleaving the alignment between the two sizes would put a
// `TypeAlignment` in the second slot and fail the match.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -dump-ir -o /dev/null -entry main -stage compute

struct Params
{
    float4 a;
    Texture2D t;
    SamplerState s;
}

ParameterBlock<Params> gPB;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = gPB.a.x;
}

// CHECK: = structTypeLayout(%[[S0:[0-9]+]], %[[S1:[0-9]+]], %[[AL:[0-9]+]], %{{[0-9]+}}
// CHECK-DAG: %[[S0]]{{.*}}= size(
// CHECK-DAG: %[[S1]]{{.*}}= size(
// CHECK-DAG: %[[AL]]{{.*}}= TypeAlignment(
