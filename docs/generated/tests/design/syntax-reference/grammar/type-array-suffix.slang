//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#types
//META: doc_section_digest=29130252ebb8566d4abf275df716b28c339ea28ecaa9f518d2ddee6bfdd47125
//META: purpose=`CoreType` accepts the array form `Type '[' Expr ']'`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the array-type spelling `int[4]` as a function-parameter
// type. We sum the elements to prove the array is the right size.

//TEST:INTERPRET(filecheck=CHECK):

int sumOf(int xs[4])
{
    return xs[0] + xs[1] + xs[2] + xs[3];
}

void main()
{
    int xs[4] = { 1, 2, 3, 4 };
    // CHECK: r=10
    printf("r=%d\n", sumOf(xs));
}
