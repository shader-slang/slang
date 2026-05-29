//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=53b6c93f0af2e9c3190efabb8af92d9e31b25ecd69db4a1c48dd5dfbd607aaa8
//META: purpose=`BreakStmt` and `ContinueStmt` parse as bare keyword forms (no label) terminating with `;`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the bare-keyword forms of `break;` and `continue;`. The
// loop skips i==2 (continue) and stops at i==4 (break).

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int sum = 0;
    for (int i = 0; i < 10; ++i)
    {
        if (i == 2) continue;
        if (i == 4) break;
        sum += i;
    }
    // CHECK: sum=4
    printf("sum=%d\n", sum);
}
