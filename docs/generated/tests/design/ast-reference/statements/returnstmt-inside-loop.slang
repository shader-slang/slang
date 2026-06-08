//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#nodes
//META: doc_section_digest=9f32f2463c3b3150d88ac333b9a732766e6f477e8e541627f6f4db0faa939a24
//META: purpose=A ReturnStmt taken inside a loop body halts the entire function; the remaining iterations of the loop and all statements after the loop do not execute.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: ReturnStmt inside a loop body. The doc says ReturnStmt
// halts the function; this must override the loop's iteration. A
// mis-lowering that treated `return` like `break` would let the
// caller observe the post-loop value of the accumulator.

//TEST:INTERPRET(filecheck=CHECK):

int findFirst(int n)
{
    for (int i = 0; i < n; i = i + 1)
    {
        if (i == 3) { return i; }
    }
    return -1;
}

void main()
{
    // CHECK: r=3
    printf("r=%d\n", findFirst(10));
    // CHECK: r2=-1
    printf("r2=%d\n", findFirst(2));
}
