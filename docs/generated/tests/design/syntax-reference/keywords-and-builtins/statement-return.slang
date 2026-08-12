//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#statement-keywords
//META: doc_section_digest=1fb3516e49ddbb5d4259751275e6b594d9d69aa0b1266c092e549ec5de2b449a
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
