//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#inlining-and-call-graph
//META: doc_section_digest=5916f676576778a4bdb5917c7cf917a7fca962142a97cb9c75be16910e0e08a2
//META: purpose=Boundary: a tiny helper WITHOUT `[ForceInline]` is NOT inlined by the default inline pass — the callee name survives in the HLSL emit. The doc claim is that `[ForceInline]` drives inlining, not size; this is the contrasting baseline.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Contrast with `inline-removes-forceinline-callee-from-emit.slang`:
// without the `[ForceInline]` attribute the inline pass does not
// inline the helper — `addOne` survives in the HLSL emit as a real
// function call.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

int addOne(int x) { return x + 1; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = addOne(a);
}

// HLSL: int addOne
// HLSL: void main
// HLSL: addOne
