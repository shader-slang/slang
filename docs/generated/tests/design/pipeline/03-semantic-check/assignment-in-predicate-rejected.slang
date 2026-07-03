//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
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
