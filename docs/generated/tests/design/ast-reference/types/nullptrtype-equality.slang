//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:09+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#pointer-reference-parameter-passing-types
//META: doc_section_digest=9632f1067419e79bbad1711bd742d58d70173e0ac1cfe04bef2332c85dbcb231
//META: purpose=NullPtrType is the singleton type of `nullptr`; two pointer locals initialised to nullptr compare equal to each other and to the literal.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe of NullPtrType: two null pointers of the same
// element type compare equal to each other (and to `nullptr`).
// Complements `nonetype-and-nullptrtype.slang` which only contrasts
// a non-null with a null.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int* p = nullptr;
    int* q = nullptr;
    // CHECK: peqnull=1
    printf("peqnull=%d\n", (p == nullptr) ? 1 : 0);
    // CHECK: peqq=1
    printf("peqq=%d\n", (p == q) ? 1 : 0);
}
