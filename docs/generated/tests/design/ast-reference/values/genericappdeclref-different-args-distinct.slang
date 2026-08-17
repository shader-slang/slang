//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#declref-family
//META: doc_section_digest=2ed6a2b41efd5c08d91ec6e84e54e1f2fbb418408ef8fd51d45ba3b049e93c89
//META: purpose=A generic decl-ref with its arguments applied is identified by the argument list, so the same generic decl applied to different arguments denotes distinct types and cross-assignment is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative half of generic-application identity: `Box<int>`
// and `Box<float>` share a generic decl but differ in their argument
// list. CHECK pins the type-mismatch diagnostic (E30019) naming both
// spellings, which proves the two applications did not collapse to one
// type despite the shared decl.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Box<T> { T v; }

void main()
{
    Box<int> bi;
    Box<float> bf = bi;
//CHECK:            ^^ expected an expression of type 'Box<float>', got 'Box<int>'
//CHECK: E30019
}
