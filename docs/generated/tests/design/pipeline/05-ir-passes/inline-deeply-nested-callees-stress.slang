//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#inlining-and-call-graph
//META: doc_section_digest=0c02024bf756789f5e2a8e8518524bff844c5a9c7658e631aab916c9a49f2d22
//META: purpose=Stress: a five-deep `[ForceInline]` call chain (h0 -> h1 -> h2 -> h3 -> h4) inlines fully — no helper name appears in the HLSL emit.
//META: intent=stress
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Call-depth stress for inline. Five `[ForceInline]` helpers chained
// together must all be inlined into `main`, leaving no helper names
// in the HLSL emit.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[ForceInline] int h0(int x) { return x + 1; }
[ForceInline] int h1(int x) { return h0(x) + 2; }
[ForceInline] int h2(int x) { return h1(x) + 3; }
[ForceInline] int h3(int x) { return h2(x) + 4; }
[ForceInline] int h4(int x) { return h3(x) + 5; }

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = h4(a + int(tid.x));
}

// HLSL: void main
// HLSL-NOT: h0
// HLSL-NOT: h1
// HLSL-NOT: h2
// HLSL-NOT: h3
// HLSL-NOT: h4
