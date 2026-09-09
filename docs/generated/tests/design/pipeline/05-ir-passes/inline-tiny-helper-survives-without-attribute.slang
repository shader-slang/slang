//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#inlining-and-call-graph
//META: doc_section_digest=0c02024bf756789f5e2a8e8518524bff844c5a9c7658e631aab916c9a49f2d22
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
