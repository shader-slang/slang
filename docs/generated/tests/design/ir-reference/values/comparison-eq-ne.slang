//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#comparison
//META: doc_section_digest=04ce765c6615b6d4afcb57668ffc53ea9c74c835ea05c4c4d58b93fe0cbf4541
//META: purpose=The `==` operator lowers to `cmpEQ` and `!=` lowers to `cmpNE`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `cmpEQ` and `cmpNE` are the equality / inequality comparison
// opcodes in the doc's Comparison table. The cross-cutting bundle
// covers `cmpGT`; this test covers the remaining equality pair.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform int b;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = select(a == b, 1, 0);
    buf[1] = select(a != b, 1, 0);
}

// IR-LABEL: func %main
// IR: cmpEQ(%a, %b)
// IR: cmpNE(%a, %b)
