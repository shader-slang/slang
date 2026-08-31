//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#transparent-members
//META: doc_section_digest=32382a9ebef19c50b9e06793af48bd9784fb821c105edada1ca8660859a10b64
//META: purpose=A tbuffer is the second buffer-block keyword that produces a transparent member, so an unqualified reference to its inner field resolves through the same transparent-plus-dereference navigation chain.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second of the two declaration keywords that create a
// transparent member. The companion cbuffer test covers the first; the
// claim is that every transparent member in a user program comes from
// one of these buffer-block forms, so the tbuffer cell has to be
// instantiated rather than assumed to follow. `g` is written without a
// qualifier even though it lives inside the anonymous struct behind
// the buffer, and the interpreter cannot host such a global, so the
// observation is the emitted HLSL: CHECK asserts the access is
// rendered as buffer, then field, then swizzle.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main

tbuffer T { float4 g; }

RWStructuredBuffer<float> outBuf;

[numthreads(1,1,1)]
void main()
{
    // CHECK: T_{{[0-9]+}}.g_{{[0-9]+}}.y
    outBuf[0] = g.y;
}
