//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#inlining-and-call-graph
//META: doc_section_digest=0c02024bf756789f5e2a8e8518524bff844c5a9c7658e631aab916c9a49f2d22
//META: purpose=Pushing a whole-element buffer load down its access chain changes the width of the read: only the leaf that is used is read, rather than a copy of the whole element.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the deferred-load footprint the row names, which is a width
// change rather than a statement move. The buffer element is a struct of
// two eight-element arrays — comfortably past the size threshold below
// which the pass leaves values alone — and only one leaf scalar is read.
// The CHECKs assert the emitted read is the chained access to that leaf
// and that no local of the whole element type is materialized, which is
// what a non-deferred whole-element load would have required.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

struct Big
{
    float a[8];
    float b[8];
}

StructuredBuffer<Big> src;
RWStructuredBuffer<float> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid: SV_DispatchThreadID)
{
    outBuf[0] = src[tid.x].a[3];
}

// The read reaches straight through to the leaf element ...
// HLSL: src_{{[0-9]+}}[tid_{{[0-9]+}}.x].a_{{[0-9]+}}[int(3)]
// ... with no whole-element temporary declared for it.
// HLSL-NOT: Big_{{[0-9]+}} _S
