//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#function-style-declarations
//META: doc_section_digest=a505032fe1ced813d290018b087d7ba67b9a672a9b25244d2cf6a5fa887fe795
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
