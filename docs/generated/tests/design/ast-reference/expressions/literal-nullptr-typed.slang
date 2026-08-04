//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=d2c639013cf026698511f9f1297a9f9de18bc449514e7113fc9483c45007d835
//META: purpose=A NullPtrLiteralExpr `nullptr` denotes the null pointer; assigning nullptr to a void* and comparing equal yields true.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// NullPtrLiteralExpr is listed in the Nodes table as `nullptr`. A
// `void*` initialized to `nullptr` compares equal to `nullptr`,
// confirming the literal's pointer-typed value.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    void* p = nullptr;
    // CHECK: eq=1
    printf("eq=%d\n", int(p == nullptr));
}
