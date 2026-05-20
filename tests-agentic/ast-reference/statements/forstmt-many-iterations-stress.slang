//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#loop-family
//META: doc_section_digest=27580c18b5b0d2c47692d85449b6144b11636a004d88adeefbe87418ecc0d12e
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
