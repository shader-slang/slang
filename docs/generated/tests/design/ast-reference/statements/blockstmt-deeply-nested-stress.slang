//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#blockstmt-and-seqstmt
//META: doc_section_digest=286b56c30cbbe8b637fe3e2feda9744698b33bf0a2434728ce750ab093cc9caf
//META: purpose=A deeply nested chain of BlockStmts (>=5 levels) each introducing its own scope still resolves shadowed names to the innermost binding; the outermost variable is preserved.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress: five levels of `{}` nesting, each redeclaring the same name.
// Verifies every block scopes its own declaration, so the innermost
// write cannot reach any outer binding. The CHECK asserts the outermost
// `v` still reads 0.

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
