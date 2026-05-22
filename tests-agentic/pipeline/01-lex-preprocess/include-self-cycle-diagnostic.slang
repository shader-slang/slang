//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/pipeline/01-lex-preprocess.md#failure-modes
//META: doc_section_digest=7b0494434454c9476d516b02e8b2bb91395544db9dcff77b2783b0b8868b34dc
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
