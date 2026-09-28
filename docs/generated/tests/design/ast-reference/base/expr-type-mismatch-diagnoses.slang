//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#expr-syntaxnode
//META: doc_section_digest=cbdd88eddf413d4b0dfa6196b2bf8c23ff2478080970ece6ed50d7450b88d1b4
//META: purpose=The checker compares an Expr's type against the surrounding context, so a call whose argument type matches no overload is diagnosed with the argument type spelled out.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies both halves of the claim: the checker has already assigned
// a type to the string-literal argument, and it uses that type to
// decide no `take` overload applies. CHECK asserts the diagnostic
// names the argument type it computed, which is only possible because
// the type travels with the expression.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void take(int v) {}
void take(float v) {}

void main()
{
    take("not-an-int-or-float");
//CHECK:^ no overload for 'take' applicable to arguments of type (String)
}
