//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#failure-modes
//META: doc_section_digest=8518a72edfb4aa449935321fb64c48a945528cb8e3f926bdb661166c52241bd1
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
// CHECK: E30019
}
