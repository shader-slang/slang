//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-code-namespace
//META: doc_section_digest=c07669430fa1f58c4db53ae38c50d1523f8399ab4e7aa698054222ac63e9d41b
//META: purpose=A self-`#include` renders E15302 with the offending path interpolated.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pin `cyclic-include` (15302) to its rendered code by triggering
// a self-include via the file's own name. The `~path` interpolation
// inserts the verbatim include argument into the rendered message.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl
#include "error-code-cyclic-include.slang"
void main() {}

// CHECK: E15302
// CHECK: cyclic `#include` of file
// CHECK: error-code-cyclic-include.slang
