//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#nodes
//META: doc_section_digest=71d352a8c5034422ec1e8eb2b4a93b52395bd85cc8305cee8432fdeb2101642c
//META: purpose=A RefParamType parameter (`__ref`) aliases the caller's storage so writes land in it, while a BorrowInParamType parameter (`__constref`) borrows it read-only.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the two borrow-flavoured passing modes against each other.
// The `__ref` callee writes through its parameter and the caller must
// see the new value; the `__constref` callee only reads. CHECK asserts
// 6 after the ref write and 7 for the borrow read, which distinguishes
// an aliasing reference from a by-value copy.

//TEST:INTERPRET(filecheck=CHECK):

void bump(__ref int x) { x = x + 1; }
int peek(__constref int x) { return x + 1; }

void main()
{
    int a = 5;
    bump(a);
    // CHECK: a=6
    printf("a=%d\n", a);
    // CHECK: p=7
    printf("p=%d\n", peek(a));
}
