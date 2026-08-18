//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#variable--binding-declarations
//META: doc_section_digest=34ae2014df6f94e86e6f1c0103e2b1bc5da580d48dd7ba67686044b7b740ee79
//META: purpose=An `'operator' OperatorName` declarator on a non-function is diagnosed as `OperatorNameOnNonFunction` (E20020).
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the restriction that goes with the previous claim: the declarator
// grammar cannot reject an operator name while reading it, so the rejection
// happens in the single choke point every C-style declarator passes through.
// `int operator+ = 10;` is a variable declarator, which is not the branch that
// grants `allowOperatorName`, so the diagnostic must fire on the operator
// token itself rather than on the initializer.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int operator+ = 10;
//CHECK:    ^ error E20020
//CHECK:    ^ an operator name can only be used to declare an operator function

void main()
{
}
