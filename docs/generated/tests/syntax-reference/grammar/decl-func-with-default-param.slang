//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#function-style-declarations
//META: doc_section_digest=1a910fb654214975ac89ecf61bf5f579c0e4a3b2724fbccb87264218145ec59b
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
