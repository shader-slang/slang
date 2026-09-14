//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/pipeline/02-parse-ast.md#failure-modes
//META: doc_section_digest=045c5379570a64224b0f0d784c24bc57410eef152e105148ecb4e662cb3f9c88
//META: purpose=The statement parser accepts only a subset of declaration forms, so a `namespace` written inside a function body is rejected with E30102 "namespace is not allowed here."
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the statement-level declaration filter. A `namespace` parses fine as
// a declaration, so nothing rejects it while the declaration itself is being
// read; the rejection comes afterwards, when the statement parser inspects what
// it got back and finds a kind outside the accepted set. The CHECK pins E30102
// and the kind-specific wording, and exhaustive mode makes "no unexpected-token
// report either" part of the assertion — which is what shows this is the
// statement filter talking rather than a syntax error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int main()
{
    namespace N { int f() { return 1; } }
//CHECK:      ^ error E30102
//CHECK:      ^ namespace is not allowed here.
    return 0;
}
