//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=b49700d31492527c686bc2846076b0dd0110de840c0102148f8bbf876b2bcdb4
//META: purpose=A binding introduced by `let` is immutable; assignment to a let-bound name is a compile-time error.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Negative test: writing to a `let`-bound name must be rejected by
// the compiler. The diagnostic mentions that the left-hand side is
// not an l-value.

//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK):

void main()
{
    let x = 7;
    x = 8;
    // CHECK-DAG: error
    // CHECK-DAG: l-value
    // CHECK-DAG: E30011
}
