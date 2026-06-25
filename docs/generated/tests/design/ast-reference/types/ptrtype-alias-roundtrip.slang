//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#pointer-reference-parameter-passing-types
//META: doc_section_digest=9632f1067419e79bbad1711bd742d58d70173e0ac1cfe04bef2332c85dbcb231
//META: purpose=PtrType<T> values can be aliased (two pointers to the same storage); a write through one pointer is observable through the other, and the pointers compare equal.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of PtrType aliasing: two pointers `p` and `q` that
// point at the same `int` local share storage. Writing through `q`
// modifies the value visible to `*p`, and `p == q` holds.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 11;
    int* p = &x;
    int* q = p;
    *q = 22;
    // CHECK: x=22
    printf("x=%d\n", x);
    // CHECK: peq=1
    printf("peq=%d\n", (p == q) ? 1 : 0);
}
