//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#failure-modes
//META: doc_section_digest=f6d63bedcd6d31bb43fb73300201d5b6af73fbcaf7464a55660a5559a1828ae0
//META: purpose=Negative probe: a `#include` of the file itself triggers the documented "cyclic include" preprocessor diagnostic.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Self-include cycle: this file includes itself by name. The
// preprocessor must detect the cycle and emit the cyclic-include
// diagnostic instead of recursing forever.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#include "include-self-cycle-diagnostic.slang"
//CHECK: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ cyclic include
//CHECK: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ cyclic `#include` of file '"include-self-cycle-diagnostic.slang"'
// CHECK: E15302

void main() {}
