//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:28:39Z
//META: source_commit=7a45a039cb7fb0a34e9184c7069a96bccdf08115
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=5749d9886287d8389af87cb5a6a536a28da48a421ac9a27f86809fca6a7b5676
//META: purpose=For-loop trip-count inference consumes the checker's resolved builtin-operator kind: a [ForceUnroll] loop whose bound uses builtin `<` and `++` compiles and runs.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the third documented consumer of the checker's
// builtin-operator resolution: for-loop trip-count inference. A
// [ForceUnroll] loop only compiles if the compiler can infer the trip
// count from the builtin `<` comparison and `++` increment; the
// printed sum 0+1+2+3=6 proves the unrolled loop ran all four
// iterations with correct operator semantics.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int sum = 0;
    [ForceUnroll]
    for (int i = 0; i < 4; i++)
    {
        sum += i;
    }
    // CHECK: sum=6
    printf("sum=%d\n", sum);
}
