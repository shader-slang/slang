//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#comparison
//META: doc_section_digest=a41eec816a55db301ccafcfa867bab048ae6ed92cc99c5f791c2e8474302d15d
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
