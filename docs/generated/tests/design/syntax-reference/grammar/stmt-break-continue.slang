//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=11f48bb43fb7900ef7dd3c2f4785da1cab7aaae9b90d2da44ecf5e8cf8e950b1
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
