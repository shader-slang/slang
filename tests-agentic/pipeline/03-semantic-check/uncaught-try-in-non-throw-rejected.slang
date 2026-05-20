//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=273b0c1d7bd60576ed4d24081c0d1b9f61952281
//META: doc_ref=docs/llm-generated/pipeline/03-semantic-check.md#files-and-responsibilities
//META: doc_section_digest=a3ca128124f08a4f3b66630a5a4d456463e6427c89255506f4853b27f329677e
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
