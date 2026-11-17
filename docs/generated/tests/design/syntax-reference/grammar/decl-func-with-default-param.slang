//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#function-style-declarations
//META: doc_section_digest=d2933f044671826cdda47d2bf6584eb3ef9b48c4c8f5e77a17b011c61c8729d4
//META: purpose=`Param` parses with an optional `'=' Expr` default-value clause.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a function parameter accepts a default-value clause
// (`Type IDENT '=' Expr`). Calling the function without that argument
// uses the default.

//TEST:INTERPRET(filecheck=CHECK):

int addOne(int x, int k = 1) { return x + k; }

void main()
{
    // CHECK: r=8
    printf("r=%d\n", addOne(7));
}
