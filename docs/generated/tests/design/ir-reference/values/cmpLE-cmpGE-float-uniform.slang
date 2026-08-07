//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#comparison
//META: doc_section_digest=04ce765c6615b6d4afcb57668ffc53ea9c74c835ea05c4c4d58b93fe0cbf4541
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
