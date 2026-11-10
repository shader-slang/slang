//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#-disambiguation
//META: doc_section_digest=9af1f98685cb612077da94d031ef06cdcc2d4768b2522616e45c9ae531a49988
//META: purpose=`<` in a body context where the preceding expression does not resolve to a generic parses as the comparison operator.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative side of the `<` disambiguation: in
// `a < b`, both operands are plain int variables, so the parser falls
// back to treating `<` as a less-than comparison.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int a = 3, b = 5;
    bool r = a < b;
    // CHECK: r=1
    printf("r=%d\n", int(r));
}
