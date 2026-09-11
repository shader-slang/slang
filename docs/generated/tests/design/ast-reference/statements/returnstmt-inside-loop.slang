//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#returnstmt
//META: doc_section_digest=c25c0b38fe3f374b6c53bf5d39f45dbef8f00f160edf334a7927f62cea89fd70
//META: purpose=A ReturnStmt taken inside a loop body halts the entire function; the remaining iterations of the loop and all statements after the loop do not execute.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: `return` taken from inside a loop body. Verifies it halts
// the whole function rather than merely leaving the loop; were it
// treated like `break`, the post-loop `return -1` would win. The CHECKs
// assert 3 for a matching input and -1 only when the loop truly ran out.

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
