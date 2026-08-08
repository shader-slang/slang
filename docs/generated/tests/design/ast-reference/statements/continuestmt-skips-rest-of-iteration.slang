//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#labelstmt-breakstmt-continuestmt-and-discardstmt
//META: doc_section_digest=7c61fa1b4009ab648f41bc90151176c2c3ef804455b39f0ca18a8fb8d9a62d07
//META: purpose=A ContinueStmt skips the remainder of the current iteration and advances to the next; statements after the `continue` are not executed for that iteration.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// ContinueStmt's documented role is to skip to the next iteration. We
// observe by counting only "non-skipped" iterations: when i is even,
// we continue before incrementing the count, so the final count
// reflects only odd values.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int oddCount = 0;
    for (int i = 0; i < 8; i = i + 1)
    {
        if ((i % 2) == 0) { continue; }
        oddCount = oddCount + 1;
    }
    // odd values in 0..7: 1, 3, 5, 7 => 4
    // CHECK: oddCount=4
    printf("oddCount=%d\n", oddCount);
}
