//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#types
//META: doc_section_digest=29130252ebb8566d4abf275df716b28c339ea28ecaa9f518d2ddee6bfdd47125
//META: purpose=The Type production includes the array form `T '[' Expr? ']'` (grammar.md handing off from the parse-AST doc's `## AST data model` Type family note); `int arr[3]` parses into an array-typed VarDecl.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Declare a 3-element `int` array, write each slot, sum them back.
// The 60 only comes out if the array-type spelling parsed and the
// three indexed assignments were recognized as l-value subscripts.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int arr[3];
    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;
    // CHECK: sum=60
    printf("sum=%d\n", arr[0] + arr[1] + arr[2]);
}
