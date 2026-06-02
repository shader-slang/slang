//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#pointer-reference-parameter-passing-types
//META: doc_section_digest=9632f1067419e79bbad1711bd742d58d70173e0ac1cfe04bef2332c85dbcb231
//META: purpose=A PtrType<T> (spelled `T*`) is a raw pointer; dereferencing it through `*p = ...` writes the pointed-to storage.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `PtrType` is the `PtrTypeBase` leaf for `T*`. The user-facing
// claim: taking the address of an int local and dereferencing the
// pointer modifies the local. Both directions (`&x` and `*p`) are
// AST observations of the PtrType.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 5;
    int* p = &x;
    *p = 17;
    // CHECK: x=17
    printf("x=%d\n", x);
}
