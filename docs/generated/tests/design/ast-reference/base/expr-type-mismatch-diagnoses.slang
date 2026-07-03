//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/base.md#expr-syntaxnode
//META: doc_section_digest=369743522fa835d335b8cfc1d7160c48df288006a4cdb5149e41fdce6b476f7e
//META: purpose=The checker fills in an Expr's QualType and then verifies it against the surrounding context; a mismatch is diagnosed at the expression's source location.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the checker uses the Expr's type to detect a call
// that has no matching overload. There is no `take(string)` overload,
// so the checker — which has already assigned a type to the string-
// literal Expr — fails to match any overload and reports an error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void take(int v) {}
void take(float v) {}

void main()
{
    take("not-an-int-or-float");
//CHECK:^ no overload for 'take' applicable to arguments of type (String)
}
