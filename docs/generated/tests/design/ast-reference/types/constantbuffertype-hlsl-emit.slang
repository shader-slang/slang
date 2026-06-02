//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#parameter-group-constant-buffer-types
//META: doc_section_digest=a62b9719fec7c122df80f8f50c81b7a71a2f4deb2062c135375ac53227244677
//META: purpose=A ConstantBufferType<T> uniform is the AST type for HLSL `cbuffer`; the HLSL emit renders it as the `cbuffer` keyword.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `ConstantBufferType` is the UniformParameterGroupType leaf for
// `ConstantBuffer<T>` / HLSL `cbuffer`. The user-facing claim is
// that a `ConstantBuffer<T>` uniform reaches the HLSL backend as
// the `cbuffer` keyword wrapping the struct.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct Params { float scale; int count; }

ConstantBuffer<Params> g_cb;
RWStructuredBuffer<float> g_out;

//CHECK: RWStructuredBuffer
//CHECK: cbuffer
[numthreads(1, 1, 1)]
void main()
{
    g_out[0] = g_cb.scale * float(g_cb.count);
}
