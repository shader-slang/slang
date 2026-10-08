//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#failure-modes
//META: doc_section_digest=f6d63bedcd6d31bb43fb73300201d5b6af73fbcaf7464a55660a5559a1828ae0
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
