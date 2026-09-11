//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#expr-syntaxnode
//META: doc_section_digest=369743522fa835d335b8cfc1d7160c48df288006a4cdb5149e41fdce6b476f7e
//META: purpose=Every Expr carries a type filled in by the checker, and that type is what overload resolution dispatches on.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the type the checker attaches to a compound expression
// (not just to a literal) is what drives overload selection. `1 + 2`
// must be dispatched as `int` and `1.0f + 2.0f` as `float`. CHECK
// asserts the two probe overloads that fire; an untyped expression
// could not be routed to either.

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
