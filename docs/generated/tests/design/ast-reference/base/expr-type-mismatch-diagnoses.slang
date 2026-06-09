//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
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

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void take(int v) {}
void take(float v) {}

void main()
{
    //CHECK: error
// CHECK: E30019
    take("not-an-int-or-float");
}
