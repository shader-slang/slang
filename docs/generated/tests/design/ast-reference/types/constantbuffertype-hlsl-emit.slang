//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=0acf92c990c51e23e6b949538b0504562447e6a609a849cd20f331b1ba8f82e3
//META: purpose=A ConstantBufferType<T> uniform makes its fields visible and is the AST type for HLSL cbuffer; the HLSL emit renders the cbuffer keyword.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies ConstantBufferType is the UniformParameterGroupType leaf
// for `ConstantBuffer<T>` / HLSL cbuffer: its fields are visible at
// the entry point and the HLSL backend renders the cbuffer keyword.
// CHECK matches the cbuffer spelling in the emitted HLSL.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct Params { float scale; int count; }

ConstantBuffer<Params> g_cb;
RWStructuredBuffer<float> g_out;

//CHECK: cbuffer
[numthreads(1, 1, 1)]
void main()
{
    g_out[0] = g_cb.scale * float(g_cb.count);
}
