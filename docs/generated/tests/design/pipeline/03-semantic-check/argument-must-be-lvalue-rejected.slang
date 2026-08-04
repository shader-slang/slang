//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
//META: purpose=Expression checking rejects passing a non-lvalue argument to an `out` parameter (E30047).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Expression-checking lvalue rule at a call site: an `out` parameter
// requires an l-value argument, and a literal is rejected with
// E30047.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void g(out int x) { x = 5; }

void main() { g(5); }
//CHECK:        ^ argument must be l-value
//CHECK:        ^ argument passed to parameter '0' must be l-value.
// CHECK: E30047
