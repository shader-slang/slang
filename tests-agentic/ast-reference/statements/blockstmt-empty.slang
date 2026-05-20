//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#blockstmt-and-seqstmt
//META: doc_section_digest=71e446148629b488a053ae16dc5e7aee850ecd11b223e54fbd724907831ae599
//META: purpose=An empty BlockStmt `{}` is accepted in statement position and has no observable effect; the surrounding state is preserved across the block.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: empty body of a BlockStmt. The doc says BlockStmt is
// `{ ... }` with a `body: Stmt*` slot; the smallest documented form
// has no statements at all. Observable consequence: the program
// compiles, and the variable visible before and after the block
// retains its value.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 42;
    {
    }
    // CHECK: x=42
    printf("x=%d\n", x);
}
