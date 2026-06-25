//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-06-24T09:20:31Z
//META: source_commit=b25e06aeb8862d207541c3d812c57cb9e381b802
//META: doc_ref=docs/generated/design/ast-reference/base.md#expr-syntaxnode
//META: doc_section_digest=c12d63b7f58fbd104856d0dee2a7e19e6d3d409167fcd2db75867e9bf5b80bd5
//META: purpose=The checker fills in an Expr's QualType and then verifies it against the surrounding context; a mismatch is diagnosed at the expression's source location.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the checker uses the Expr's type to detect a call
// that has no matching overload. There is no `take(string)` overload,
// so the checker — which has already assigned a type to the string-
// literal Expr — fails to match any overload and reports an error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void take(int v) {}
void take(float v) {}

void main()
{
    take("not-an-int-or-float");
//CHECK:^ no overload for 'take' applicable to arguments of type (String)
}
