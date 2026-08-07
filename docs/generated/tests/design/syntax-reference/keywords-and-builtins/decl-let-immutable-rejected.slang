//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/syntax-reference/keywords-and-builtins.md#decl-keywords
//META: doc_section_digest=e3d06ef8bf7a4508b0ecb0515b3867979078821ac9b8143e7b039cd76e1b73b1
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
