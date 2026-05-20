//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#blockstmt-and-seqstmt
//META: doc_section_digest=71e446148629b488a053ae16dc5e7aee850ecd11b223e54fbd724907831ae599
//META: purpose=A deeply nested chain of BlockStmts (>=5 levels) each introducing its own scope still resolves shadowed names to the innermost binding; the outermost variable is preserved.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress: 5-deep BlockStmt nesting. The doc says each BlockStmt is a
// ScopeStmt with its own ScopeDecl. At each level we redeclare a
// local with the same name; the innermost write must not leak to
// any outer level. The outermost binding's value is the observable.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int v = 0;
    {
        int v = 1;
        {
            int v = 2;
            {
                int v = 3;
                {
                    int v = 4;
                    {
                        int v = 5;
                        v = v + 100;
                    }
                }
            }
        }
    }
    // CHECK: v=0
    printf("v=%d\n", v);
}
