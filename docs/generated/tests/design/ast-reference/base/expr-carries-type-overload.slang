//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/ast-reference/base.md#expr-syntaxnode
//META: doc_section_digest=c12d63b7f58fbd104856d0dee2a7e19e6d3d409167fcd2db75867e9bf5b80bd5
//META: purpose=Every Expr carries a QualType filled in by the checker; that type is what overload resolution dispatches on.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an expression's type, attached to the Expr by the
// checker, is what selects an overload. The expression `1.0f + 2.0f`
// must carry type `float` and pick the float overload; `1 + 2` must
// carry type `int` and pick the int overload. If Expr did not carry
// the checked type, the program could not be type-dispatched.

//TEST:INTERPRET(filecheck=CHECK):

int which(int v)   { return 1; }
int which(float v) { return 2; }

void main()
{
    // CHECK: int-expr=1
    printf("int-expr=%d\n",   which(1 + 2));
    // CHECK: float-expr=2
    printf("float-expr=%d\n", which(1.0f + 2.0f));
}
