//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
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
