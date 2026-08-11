//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=A PrefixExpr `-x` is a unary prefix operator that negates an integer value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// PrefixExpr is documented as the AST kind for unary prefix operators
// (`-a`, `!x`). Applying `-` to a positive int must yield its
// negation; applying `-` twice returns the original value.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 7;
    int negx = -x;
    int negnegx = -(-x);
    // CHECK: neg=-7
    printf("neg=%d\n", negx);
    // CHECK: negneg=7
    printf("negneg=%d\n", negnegx);
}
