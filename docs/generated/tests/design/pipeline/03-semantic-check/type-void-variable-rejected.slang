//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=b5f523113b24470cb3eee4f9c8ae76906f41333e955cb7bb7bac14ed1e5d3a05
//META: purpose=Type checking resolves the type expression of a declaration and rejects `void` as a variable type with E30009.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "resolves Type references that appear in Expr form" concern:
// `void` parses as an ordinary type expression and is only rejected once the
// type checker has resolved it and applied the variable-type rule. CHECK pins
// E30009 on the type token.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    void v;
//CHECK: E30009
}
