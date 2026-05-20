//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#nodes
//META: doc_section_digest=9f32f2463c3b3150d88ac333b9a732766e6f477e8e541627f6f4db0faa939a24
//META: purpose=A ReturnStmt in a void function is allowed to omit its expression (`return;`); the function halts and statements after the bare return do not execute on the early-return path.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: bare `return;` form. The doc spells ReturnStmt as
// "`return` (optionally with an expression)". The void-function
// side of the optionality is accepted; the partner negative test
// rejects the non-void side. We observe the halting effect on the
// early-exit path: only the pre-return print fires when the int
// argument signals the early return.

//TEST:INTERPRET(filecheck=CHECK):

void worker(int n)
{
    printf("entry n=%d\n", n);
    if (n < 0) { return; }
    printf("post-return n=%d\n", n);
}

void main()
{
    worker(5);
    worker(-1);
    // CHECK: entry n=5
    // CHECK-NEXT: post-return n=5
    // CHECK-NEXT: entry n=-1
}
