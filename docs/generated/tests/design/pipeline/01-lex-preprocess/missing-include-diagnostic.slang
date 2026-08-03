//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#failure-modes
//META: doc_section_digest=ae4aa1fa234c5a91862002d52c8aeecec1baa813ca385b39da9d91ae3c677ffe
//META: purpose=A `#include` whose target cannot be resolved emits a preprocessor diagnostic through the DiagnosticSink.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an `#include` of a non-existent file is reported as
// a diagnostic. We FileCheck for the "failed to find include file"
// message text in the diagnostic stream.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#include "this-file-does-not-exist.slang.h"
//CHECK: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ include file not found
//CHECK: ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ failed to find include file
// CHECK: E15300

void main() {}
