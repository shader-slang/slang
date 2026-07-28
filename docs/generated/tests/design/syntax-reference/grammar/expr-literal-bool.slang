//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:43:29Z
//META: source_commit=196bcf98f30aef1230ca33a5bada98c48c9cb7fb
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#literal-forms-vs-token-kinds
//META: doc_section_digest=c7d6780d315f67918c23ddaaf2cd151fb568fe783747ef75230b9c640ed1629f
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
