//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/syntax-reference/grammar.md#types
//META: doc_section_digest=0ce0b3e0544deb35556a1e8955b041223e597dbd359e81100c105e6f289f6efa
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
