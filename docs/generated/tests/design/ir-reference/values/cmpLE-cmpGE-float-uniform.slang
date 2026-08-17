//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#comparison
//META: doc_section_digest=a41eec816a55db301ccafcfa867bab048ae6ed92cc99c5f791c2e8474302d15d
//META: purpose=Boundary: `<=` / `>=` on `uniform float` operands lower to `cmpLE` / `cmpGE` — verifying the ordered-comparison opcodes survive on the float type (existing smoke test covers integer ordering).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The existing `comparison-lt-le-ge.slang` smoke test exercises
// `cmpLT` / `cmpLE` / `cmpGE` on integers. This boundary probe
// runs the ordered comparisons on `float` operands to verify the
// same opcodes are produced (the doc's Comparison table does not
// list separate float opcodes).

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform float fb;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = select(fa <= fb, 1, 0);
    buf[1] = select(fa >= fb, 1, 0);
}

// IR-LABEL: func %main
// IR: cmpLE(%fa, %fb)
// IR: cmpGE(%fa, %fb)
