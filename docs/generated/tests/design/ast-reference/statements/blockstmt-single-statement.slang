//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#blockstmt-and-seqstmt
//META: doc_section_digest=71e446148629b488a053ae16dc5e7aee850ecd11b223e54fbd724907831ae599
//META: purpose=A BlockStmt containing a single statement executes that statement and introduces a scope; a local declared inside is not visible after the block closes.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: a BlockStmt whose body slot holds exactly one statement
// (no SeqStmt is required). The doc says BlockStmt introduces a
// ScopeDecl; even with a single declaration inside, the name must
// be confined to the block body.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int outer = 7;
    {
        int outer = 99;
    }
    // CHECK: outer=7
    printf("outer=%d\n", outer);
}
