//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
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
