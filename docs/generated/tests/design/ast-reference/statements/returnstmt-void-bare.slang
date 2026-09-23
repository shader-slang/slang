//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#returnstmt
//META: doc_section_digest=c25c0b38fe3f374b6c53bf5d39f45dbef8f00f160edf334a7927f62cea89fd70
//META: purpose=A ReturnStmt in a void function is allowed to omit its expression (`return;`); the function halts and statements after the bare return do not execute on the early-return path.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the returned expression may be omitted when the enclosing
// function returns `void`, and that the bare `return` still halts the
// function. The CHECKs assert the post-return print fires only on the
// path that did not take the early return.

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
