//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#nodes
//META: doc_section_digest=9f32f2463c3b3150d88ac333b9a732766e6f477e8e541627f6f4db0faa939a24
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
