//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=3e3c1fa508130c56014c50d7c74ee56f2e557c277dfbc86c712dbb9373bf794a
//META: purpose=Boundary: a `bool` field on a local struct routed through type-legalization emits a comparison-shaped result on HLSL — bool packing does not corrupt the live read.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `bool` is a tricky type for legalization — different targets pack
// it differently. We write to a struct field of type `bool` and read
// it back through a ternary that feeds the output buffer. The
// `bool` keyword (HLSL) and `bvec`/comparison spelling (SPIR-V) both
// confirm the field survived legalization.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

struct Flags { bool live; int n; };

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    Flags f;
    f.n = a;
    f.live = (a > 0);
    buf[0] = f.live ? f.n : -f.n;
}

// HLSL: bool
// HLSL: void main
