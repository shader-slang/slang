//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=0acf92c990c51e23e6b949538b0504562447e6a609a849cd20f331b1ba8f82e3
//META: purpose=An HLSLStructuredBufferType (read-only StructuredBuffer<T>) is accepted at a uniform and its elements are addressable; the HLSL emit renders the StructuredBuffer spelling.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies HLSLStructuredBufferType is the read-only structured
// buffer leaf: elements are readable and the HLSL backend renders the
// `StructuredBuffer` spelling distinct from its RW sibling. CHECK
// matches both the read-only and RW spellings in the emitted HLSL.

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
