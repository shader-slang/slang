//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#the-major-node-families
//META: doc_section_digest=bf6116c2e5b561cd38063b60dd513143618c8f41f5c801a0b352865012956ca2
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
