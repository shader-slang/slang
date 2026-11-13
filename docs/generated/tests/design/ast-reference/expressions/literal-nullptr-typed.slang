//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#literalexpr-family
//META: doc_section_digest=a94dca0039eb4252e1b8fd3b7f350745e39efaac5fece1dae4ad82b3e6325392
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
