//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=c07669430fa1f58c4db53ae38c50d1523f8399ab4e7aa698054222ac63e9d41b
//META: purpose=A missing `#include` file renders E15300 with the requested path interpolated.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pin `include-failed` (15300) to its rendered code by including a
// path that does not exist on disk. The diag is preprocessor-stage
// and has its own id distinct from cyclic-include (15302).

//TEST:SIMPLE(filecheck=CHECK):-target hlsl
#include "this-file-does-not-exist-anywhere-12345.slang"
void main() {}

// CHECK: E15300
// CHECK: include file not found
