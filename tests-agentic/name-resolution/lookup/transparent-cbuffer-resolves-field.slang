//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T13:48:38Z
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/name-resolution/lookup.md#transparent-members
//META: doc_section_digest=ac9ff1bb7fdc57f330cb4aab07ddae7ce2c0b3db8c51e76a2dcc91b672f6a7a9
//META: purpose=An HLSL cbuffer is lowered to a __transparent ConstantBuffer; an unqualified reference to its inner field resolves through the transparent (Member) + deref breadcrumb chain.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's canonical transparent-member example is
// `cbuffer C { float4 f; }` lowered to `__transparent
// ConstantBuffer<Anon0> anon1`. Unqualified `f` must resolve via a
// Deref + Member breadcrumb chain through `anon1`. We observe that
// resolution by writing emitted HLSL: the unqualified `f.x` reads
// through the lowered cbuffer's field. INTERPRET cannot host
// `cbuffer` (it asks for VM bytecode for unsupported globals), so
// use a text-emit target.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main

cbuffer C { float4 f; }

RWStructuredBuffer<float> outBuf;

[numthreads(1,1,1)]
void main()
{
    // The resolved access must surface the transparent field;
    // mangling decorates the name but the field identifier survives.
    // CHECK: f_{{[0-9]+}}.x
    outBuf[0] = f.x;
}
