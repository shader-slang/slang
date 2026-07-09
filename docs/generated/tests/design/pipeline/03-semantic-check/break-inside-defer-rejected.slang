//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
//META: purpose=Statement checking rejects `break` inside a `defer` block (E30108).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Statement-checking boundary: `defer` has its own scope rules,
// disallowing `break` (E30108) even when the defer is inside a
// loop that would otherwise make `break` legal.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    for (int i = 0; i < 10; i++) { defer { break; } }
//CHECK:                                   ^^^^^ 'break' inside defer
//CHECK:                                   ^^^^^ 'break' must not appear inside a defer statement.
// CHECK: E30108
}
