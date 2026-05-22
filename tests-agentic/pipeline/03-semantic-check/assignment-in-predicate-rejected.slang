//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
//META: purpose=Expression checking rejects a bare assignment used as the predicate of an `if` statement (E30057).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Statement-checking boundary: assignment-in-predicate (`if (x = 5)`)
// is rejected without explicit `()` parens.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main() { int x = 0; if (x = 5) { } }
//CHECK:                       ^ assignment in predicate expression not allowed
//CHECK:                       ^ use an assignment operation as predicate expression is not allowed, wrap the assignment with '()' to clarify the intent.
// CHECK: E30057
