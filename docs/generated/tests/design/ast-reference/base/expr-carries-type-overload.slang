//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/base.md#expr-syntaxnode
//META: doc_section_digest=369743522fa835d335b8cfc1d7160c48df288006a4cdb5149e41fdce6b476f7e
//META: purpose=Every Expr carries a QualType filled in by the checker; that type is what overload resolution dispatches on.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an expression's type, attached to the Expr by the
// checker, is what selects an overload. The expression `1 + 2` must
// carry type `int` and pick the int overload; `1.0f + 2.0f` must
// carry type `float` and pick the float overload. If Expr did not
// carry the checked type, the program could not be type-dispatched.

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
