//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=f15f833400e70f33bd4f151e90be63f4887ec974cd69a44d2663e7a0d2ca3bdb
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
