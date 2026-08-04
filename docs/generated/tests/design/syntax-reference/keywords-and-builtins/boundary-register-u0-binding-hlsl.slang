//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:30:00+00:00
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#core-module-syntax-declarations
//META: doc_section_digest=950d7a7efcea5da748ec49fa6499555097f3093abb07ac97945a51be1ce2339c
//META: purpose=Boundary: HLSL `register(uN)` UAV-binding annotation is part of the HLSL-flavored vocabulary contributed by `hlsl.meta.slang`; it survives into HLSL emit attached to the corresponding buffer.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Probes the `register(uN)` slot-binding boundary at slot 0 (the
// documented lower edge). The HLSL emit reproduces the annotation
// verbatim.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> outBuf : register(u0);

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    //CHECK: register(u0)
    outBuf[0] = 7;
}
