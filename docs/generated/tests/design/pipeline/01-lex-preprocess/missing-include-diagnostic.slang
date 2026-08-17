//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#failure-modes
//META: doc_section_digest=b47fb1d2e5530bf02a1606e2006144db0e35da148f564b5370d15c8fe13ad12f
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
