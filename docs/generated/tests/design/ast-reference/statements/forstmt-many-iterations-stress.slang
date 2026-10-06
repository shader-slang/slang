//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#loop-family
//META: doc_section_digest=e6660f713acc83bad477b54d0f5111af9b9f300c42158a5d582ea3f003dc7eff
//META: purpose=A ForStmt iterated 100 times accumulates the documented count; the loop variable advances monotonically and the side-effect expression runs once per iteration.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress: 100-iteration ForStmt. Sum of 0..99 is 4950. A bug that
// dropped, doubled, or skipped iterations would mis-sum.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int sum = 0;
    for (int i = 0; i < 100; i = i + 1)
    {
        sum = sum + i;
    }
    // CHECK: sum=4950
    printf("sum=%d\n", sum);
}
