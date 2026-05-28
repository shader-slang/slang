//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
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
