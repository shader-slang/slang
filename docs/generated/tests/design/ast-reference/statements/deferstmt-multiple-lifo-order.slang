//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#deferstmt
//META: doc_section_digest=9d7a17de2e7a4cacf7a44eefe78eaa8bea0a9a617b555cc1e20d4e784f62d6bf
//META: purpose=Multiple DeferStmt inner statements written in a single scope all run when that scope exits, after the surrounding body's own statements.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies several deferred statements in one scope all run when that
// scope exits. Three defers are written before the block's own print;
// the CHECKs assert the block body printed first, then three deferred
// prints, then the statement after the block.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    {
        defer printf("d1\n");
        defer printf("d2\n");
        defer printf("d3\n");
        printf("body\n");
    }
    printf("after\n");
    // CHECK: body
    // CHECK: d
    // CHECK: d
    // CHECK: d
    // CHECK: after
}
