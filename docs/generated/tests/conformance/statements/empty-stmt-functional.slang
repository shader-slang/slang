//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T07:58:55+00:00
//META: source_commit=a01e79278cf0be9772deb8b2be52ee87687697a3
//META: doc_ref=docs/language-reference/statements.md#empty-statement
//META: doc_section_digest=e24d5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0c1d2e3f4a5b6c7d8e9f0a1b2c3d4
//META: purpose=A lone semicolon is a valid empty statement; used as for-loop body with zero iterations.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a lone `;` is accepted as an empty statement in a for-loop body position.
// The loop counts to N using `;` as the body; the update variable tracks loop progress.
//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int i;
    // Empty statement as for-loop body: loop completes, i reaches 5
    for (i = 0; i < 5; i++);
    // CHECK: 5
    printf("%d\n", i);
}
