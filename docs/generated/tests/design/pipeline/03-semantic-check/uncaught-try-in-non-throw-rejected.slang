//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=dfab593613903d0888a3e07660f6118db1ff7f0717e05368cfb4b5561eae574b
//META: purpose=Statement checking rejects an uncaught `try` inside a function not declared `throws` (E30093).
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Error-propagation boundary: `try` in a non-throwing function is
// not allowed unless caught.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int g() throws int { throw 1; return 0; }
void main() { try g(); }
//CHECK:      ^^^ uncaught 'try' in non-throwing function
//CHECK:      ^^^ the current function or environment is not declared to throw any errors, but the 'try' clause is not caught
// CHECK: E30093
