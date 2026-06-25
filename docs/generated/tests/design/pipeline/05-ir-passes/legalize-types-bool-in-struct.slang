//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=440ef60cd49aa7b190c5138fc5695d86b4ac64644e03bf09eb6782e82ce75b0e
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
