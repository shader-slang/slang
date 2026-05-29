//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#parameter-group-constant-buffer-types
//META: doc_section_digest=a62b9719fec7c122df80f8f50c81b7a71a2f4deb2062c135375ac53227244677
//META: purpose=ConstantBufferType<T> with a single-scalar payload (struct of one float) still lowers to HLSL cbuffer at emit; the minimum payload exercises the small edge of the parameter-group machinery.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe at the small-payload edge of ConstantBufferType:
// the struct contains a single scalar field. The HLSL backend
// still wraps it in a cbuffer block. Complements the existing
// two-field smoke (constantbuffertype-hlsl-emit).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

struct Single { float v; }

ConstantBuffer<Single> g_cb;
RWStructuredBuffer<float> g_out;

//CHECK: cbuffer
[numthreads(1, 1, 1)]
void main()
{
    g_out[0] = g_cb.v;
}
