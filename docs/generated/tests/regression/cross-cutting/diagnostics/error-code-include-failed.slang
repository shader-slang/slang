//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=d245d3be9942f0847c45c138b4c2780e7df19ecb
//META: doc_ref=docs/generated/design/cross-cutting/diagnostics.md#error-codes-and-the-name-field
//META: doc_section_digest=58de6e7299bd58eba9a686426fc010a16e2745f7e3231042be33437e83834630
//META: purpose=Negative: an `#include` of a missing file renders E15300 with the documented "include file not found" message.
//META: intent=negative
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
