//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=3571737d4bbbec12cece17f18e8198029c1f511e52b3e3c93fe529db89cc2fc1
//META: purpose=`return <expr>` is a statement keyword that hands a value back to the caller and ends the function.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The early return on the negative branch verifies that `return` does
// not just compute the expression but also halts further execution of
// the function body.

//TEST:INTERPRET(filecheck=CHECK):

int firstNonNeg(int a, int b)
{
    if (a >= 0) { return a; }
    return b;
}

void main()
{
    // CHECK: x=7
    printf("x=%d\n", firstNonNeg(-1, 7));
    // CHECK: y=3
    printf("y=%d\n", firstNonNeg(3, -1));
}
