//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T22:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ast-reference/types.md#pointer-reference-parameter-passing-types
//META: doc_section_digest=9632f1067419e79bbad1711bd742d58d70173e0ac1cfe04bef2332c85dbcb231
//META: purpose=NullPtrType is the type of `nullptr` (compares equal to a null T*); NoneType is the type of `none` (the empty Optional).
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `NullPtrType` and `NoneType` are the two singleton BuiltinType
// nodes for literal-of-no-value. Observed by comparison: `nullptr`
// matches a null `T*`, while a non-null pointer does not.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 9;
    int* p = &x;
    int* q = nullptr;
    // CHECK: p_nz=1
    printf("p_nz=%d\n", (p != nullptr) ? 1 : 0);
    // CHECK: q_nz=0
    printf("q_nz=%d\n", (q != nullptr) ? 1 : 0);
}
