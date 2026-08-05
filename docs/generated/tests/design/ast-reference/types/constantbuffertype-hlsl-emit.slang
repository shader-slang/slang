//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#resource-and-texture-type-families
//META: doc_section_digest=0036091ab5f0cd50c2bada4820b7c7d7db668e207cf8e5052d6b074f59b4585f
//META: purpose=A ConstantBufferType `ConstantBuffer<T>` makes its element's fields visible at the entry point and lowers to an HLSL cbuffer.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the uniform-parameter-group leaf for constant buffers: the
// element struct's fields are read straight off the buffer value, and
// the HLSL backend must render the binding as a cbuffer rather than as
// a plain struct parameter. CHECK matches the cbuffer keyword in the
// emitted HLSL.

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
