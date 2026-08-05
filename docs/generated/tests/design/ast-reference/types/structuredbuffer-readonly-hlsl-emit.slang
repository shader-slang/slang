//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#resource-and-texture-type-families
//META: doc_section_digest=0036091ab5f0cd50c2bada4820b7c7d7db668e207cf8e5052d6b074f59b4585f
//META: purpose=An HLSLStructuredBufferType `StructuredBuffer<T>` is a distinct read-only buffer type from its RW sibling and keeps that spelling through HLSL emission.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the read-only and read-write structured buffer families
// are separate types rather than one type with a flag: the same shader
// declares both and copies between them. CHECK anchors the read-only
// spelling at line start so it cannot be satisfied by the `RW` prefix
// of its sibling.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

StructuredBuffer<int> g_in;
RWStructuredBuffer<int> g_out;

//CHECK-DAG: {{^}}StructuredBuffer<int
//CHECK-DAG: RWStructuredBuffer<int
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    g_out[tid.x] = g_in[tid.x];
}
