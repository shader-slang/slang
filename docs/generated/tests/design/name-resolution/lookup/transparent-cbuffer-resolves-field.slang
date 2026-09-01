//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#transparent-members
//META: doc_section_digest=32382a9ebef19c50b9e06793af48bd9784fb821c105edada1ca8660859a10b64
//META: purpose=An HLSL cbuffer lowers to a transparent ConstantBuffer of an anonymous struct, so an unqualified reference to its inner field resolves through the transparent-member plus dereference navigation chain.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the canonical transparent-member example. `f` is written
// unqualified even though it lives inside the anonymous struct behind
// the lowered constant buffer, so resolving it requires the transparent
// injection step followed by the pointer dereference. INTERPRET cannot
// host a cbuffer global, so the observation is the emitted HLSL: CHECK
// asserts the access is rendered as buffer, then field, then swizzle —
// exactly the two-step navigation lookup recorded.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main

cbuffer C { float4 f; }

RWStructuredBuffer<float> outBuf;

[numthreads(1,1,1)]
void main()
{
    // CHECK: C_{{[0-9]+}}.f_{{[0-9]+}}.x
    outBuf[0] = f.x;
}
