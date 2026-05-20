//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:00:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=ba8996c1b140750bb2e807ee33d646c0a62d626b5b41e86bf40fe00640f5b270
//META: purpose=A type mismatch at a coercion site is rejected with the type-mismatch (30019) diagnostic citing the expected and actual types.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Failure modes section says diagnostics flow through the
// DiagnosticSink. Verify the type-mismatch diagnostic fires at a
// coercion site with the wrong type and that both expected and
// actual types appear in the rendered message.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct A {};
struct B {};

void takeA(A a) {}

void main()
{
    B b;
    takeA(b);
//CHECK:  ^ type mismatch in expression
//CHECK:  ^ expected an expression of type 'A', got 'B'
}
