//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#literal-forms-vs-token-kinds
//META: doc_section_digest=f5c669bb9740dfa642523af3da2be0ac7b679950ab4f326668ed87a90288d1e6
//META: purpose=`true` and `false` are recognized as `Literal` atoms (BoolLiteralExpr) despite lexing as identifiers.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the identifier-spelled bool literals: `true` and `false`
// participate directly in a boolean expression whose value drives the
// printed result, so both spellings must have parsed as literal atoms.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    let t = true;
    let f = false;
    int r = (t && !f) ? 1 : 0;
    // CHECK: r=1
    printf("r=%d\n", r);
}
