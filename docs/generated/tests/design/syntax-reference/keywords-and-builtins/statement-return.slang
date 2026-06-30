//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=d6e74209f0cfcbd939127caa9bf2791387db9155c2d0b2f0a561349e2e185456
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
